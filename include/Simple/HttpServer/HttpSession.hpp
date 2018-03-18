#ifndef SIMPLE_HTTPSERVER_HTTPSESSION_HPP
#define SIMPLE_HTTPSERVER_HTTPSESSION_HPP

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>

#include "LoggingHelper.hpp"

namespace Simple::HttpServer {

using tcp = boost::asio::ip::tcp;
namespace websocket = boost::beast::websocket;
namespace http = boost::beast::http;

template <typename Context, typename RequestHandler, typename WebsocketSession>
class HttpSession
    : public std::enable_shared_from_this<HttpSession<Context, RequestHandler, WebsocketSession>> {
    class Queue {
        enum {
            // Maximum number of responses we will queue
            limit = 8
        };

        // The type-erased, saved work item
        struct Work {
            virtual ~Work() = default;
            virtual void operator()() = 0;
        };

        HttpSession& session_;
        std::vector<std::unique_ptr<Work>> items_;

    public:
        Queue(HttpSession& session)
            : session_(session)
        {
        }

        // Returns `true` if we have reached the queue limit
        bool isFull() const
        {
            return items_.size() >= limit;
        }

        // Called when a message finishes sending
        // Returns `true` if the caller should initiate a read
        bool onWrite()
        {
            BOOST_ASSERT(!items_.empty());
            auto const was_full = isFull();
            items_.erase(items_.begin());
            if (!items_.empty())
                (*items_.front())();
            return was_full;
        }

        template <bool isRequest, class Body, class Fields>
        void operator()(http::message<isRequest, Body, Fields>&& msg)
        {
            // This holds a work item
            struct WorkImpl : Work {
                HttpSession& session_;
                http::message<isRequest, Body, Fields> msg_;

                WorkImpl(HttpSession& session, http::message<isRequest, Body, Fields>&& msg)
                    : session_(session)
                    , msg_(std::move(msg))
                {
                }

                void operator()()
                {
                    http::async_write(session_.socket_, msg_,
                                      session_.strand_.wrap(std::bind(&HttpSession::onWrite,
                                                                      session_.shared_from_this(),
                                                                      std::placeholders::_1)));
                }
            };

            // Allocate and store the work
            items_.emplace_back(new WorkImpl(session_, std::move(msg)));

            // If there was no previous work, start this one
            if (items_.size() == 1)
                (*items_.front())();
        }
    };

    tcp::socket socket_;
    std::string const& docRoot_;
    Context& context_;
    RequestHandler handler_;
    boost::asio::io_service::strand strand_;
    Queue queue_;
    boost::beast::flat_buffer buffer_;
    http::request<http::string_body> req_;
    http::response<http::string_body> res_;

public:
    HttpSession(tcp::socket socket, std::string const& docRoot, Context& context)
        : socket_{std::move(socket)}
        , docRoot_{docRoot}
        , context_{context}
        , handler_{context}
        , strand_{socket_.get_io_service()}
        , queue_{*this}
    {
    }

    void run()
    {
        doRead();
    }

private:
    void doRead()
    {
        // Read a request
        http::async_read(socket_, buffer_, req_,
                         strand_.wrap(std::bind(&HttpSession::onRead, this->shared_from_this(),
                                                std::placeholders::_1)));
    }

    void onRead(boost::system::error_code ec)
    {
        // Happens when the timer closes the socket
        if (ec == boost::asio::error::operation_aborted)
            return;

        // This means they closed the connection
        if (ec == http::error::end_of_stream)
            return doClose();

        if (ec)
            return fail(ec, "read");

        // See if it is a WebSocket Upgrade
        if (websocket::is_upgrade(req_)) {
            // Create a WebSocket websocket_session by transferring the socket
            std::make_shared<WebsocketSession>(std::move(socket_), context_)->run(std::move(req_));
            return;
        }

        // Send the response
        handler_(docRoot_, std::move(req_), queue_);

        if (!queue_.isFull())
            doRead();
    }

    void onWrite(boost::system::error_code ec)
    {
        // Happens when the timer closes the socket
        if (ec == boost::asio::error::operation_aborted)
            return;

        if (ec == http::error::end_of_stream) {
            // This means we should close the connection, usually because
            // the response indicated the "Connection: close" semantic.
            return doClose();
        }

        if (ec)
            return fail(ec, "write");

        // Inform the queue that a write completed
        if (queue_.onWrite()) {
            // Read another request
            doRead();
        }
    }

    void doClose()
    {
        // Send a TCP shutdown
        boost::system::error_code ec;
        socket_.shutdown(tcp::socket::shutdown_send, ec);

        // At this point the connection is closed gracefully
    }
};

} // namespace Simple::HttpServer
#endif
