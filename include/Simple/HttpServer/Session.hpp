#ifndef SIMPLE_HTTP_SESSION_HPP
#define SIMPLE_HTTP_SESSION_HPP

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <functional>

#include "Common.hpp"
#include "Connection.hpp"
#include "LoggingHelper.hpp"
#include "MimeType.hpp"

namespace Simple::Http {

namespace details {
template <class T>
struct remove_first_from_tuple;

template <class T, class... Ts>
struct remove_first_from_tuple<std::tuple<T, Ts...>> {
    using Type = std::tuple<Ts...>;
};

// find type of first tuple param
template <class T>
struct type_of_first;
template <class T, class... Ts>
struct type_of_first<std::tuple<T, Ts...>> {
    using Type = T;
};

template <size_t offset, size_t index, class... TBigger, class... TSmaller>
void copy_tuple_params(std::tuple<TBigger...>& bigger, std::tuple<TSmaller...>& smaller)
{
    std::get<index>(smaller) = std::get<index + offset>(bigger);
    if constexpr (index != 0) {
        copy_tuple_params<offset, index - 1>(bigger, smaller);
    }
}

template <class... Ts, class Smaller = typename remove_first_from_tuple<std::tuple<Ts...>>::Type>
Smaller make_tuple_without_first(std::tuple<Ts...>& bigger)
{
    Smaller smaller;
    copy_tuple_params<1, sizeof...(Ts) - 2>(bigger, smaller);
    return std::move(smaller);
}

} // namespace details

class Session : public std::enable_shared_from_this<Session> {
    using WebsocketCallbacks = Connection::Callbacks;

    // This is the C++11 equivalent of a generic lambda.
    // The function object is used to send an HTTP message.
    struct SendLambda {
        Session& self_;

        explicit SendLambda(Session& self)
            : self_(self)
        {
        }

        template <bool isRequest, class Body, class Fields>
        void operator()(http::message<isRequest, Body, Fields>&& msg) const
        {
            // The lifetime of the message has to extend
            // for the duration of the async operation so
            // we use a shared_ptr to manage it.
            auto sp = std::make_shared<http::message<isRequest, Body, Fields>>(std::move(msg));

            // Store a type-erased version of the shared
            // pointer in the class to keep it alive.
            self_.res_ = sp;

            // Write the response
            http::async_write(
                self_.socket_, *sp,
                boost::asio::bind_executor(self_.strand_,
                                           std::bind(&Session::onWrite, self_.shared_from_this(), std::placeholders::_1,
                                                     std::placeholders::_2, sp->need_eof())));
        }
    };

    tcp::socket socket_;
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;
    boost::beast::flat_buffer buffer_;
    http::request<http::string_body> req_;
    std::shared_ptr<void> res_;
    SendLambda lambda_;
    RequestCallback reqCallback_;
    WebsocketCallbacks wsCallbacks_;

public:
    Session(tcp::socket socket, Callbacks callbacks)
        : socket_{std::move(socket)}
        , strand_{socket_.get_executor()}
        , lambda_{*this}
        , reqCallback_{std::get<0>(callbacks)}
        , wsCallbacks_{details::make_tuple_without_first(callbacks)}
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
        req_ = {};
        http::async_read(socket_, buffer_, req_,
                         boost::asio::bind_executor(strand_, std::bind(&Session::onRead, shared_from_this(),
                                                                       std::placeholders::_1, std::placeholders::_2)));
    }

    void onRead(boost::system::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        // Happens when the timer closes the socket
        if (ec == boost::asio::error::operation_aborted) {
            return;
        }

        // This means they closed the connection
        if (ec == http::error::end_of_stream) {
            return doClose();
        }

        if (ec) {
            return fail(ec, "read");
        }

        // See if it is a WebSocket Upgrade
        if (websocket::is_upgrade(req_)) {
            // Create a WebSocket websocket_session by transferring the socket
            std::make_shared<Connection>(std::move(socket_), wsCallbacks_)->run(std::move(req_));
            return;
        }

        // if callback is set pass request to callback
        Response response;
        if (reqCallback_ != nullptr) {
            response = reqCallback_(std::as_const(req_));
        }
        else {
            // if not callback is set, threat request as file request
            response.file = req_.target().to_string();
            response.type = Response::Type::File;
        }

        auto const not_found = [this](boost::beast::string_view target) {
            http::response<http::string_body> res{http::status::not_found, req_.version()};
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, "text/html");
            res.keep_alive(req_.keep_alive());
            res.body() = "The resource '" + target.to_string() + "' was not found.";
            res.prepare_payload();
            return res;
        };

        auto const string_response = [this](const std::string& text, const std::string& content_type) {
            http::response<http::string_body> res{http::status::ok, req_.version()};
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, content_type);
            res.keep_alive(req_.keep_alive());
            res.body() = text;
            res.prepare_payload();
            return res;
        };

        auto const file_response = [this](boost::beast::string_view path, http::file_body::value_type&& body) {
            http::response<http::file_body> res{std::piecewise_construct, std::make_tuple(std::move(body)),
                                                std::make_tuple(http::status::ok, req_.version())};
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, getMimeType(path));
            res.content_length(body.size());
            res.keep_alive(req_.keep_alive());
            return res;
        };

        auto const server_error = [this](boost::beast::string_view what) {
            http::response<http::string_body> res{http::status::internal_server_error, req_.version()};
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, "text/html");
            res.keep_alive(req_.keep_alive());
            res.body() = "An error occurred: '" + what.to_string() + "'";
            res.prepare_payload();
            return res;
        };

        // if type of response is file
        if (response.type == Response::Type::File) {
            // Attempt to open the file
            boost::beast::error_code ec;
            http::file_body::value_type body;
            body.open(response.file.c_str(), boost::beast::file_mode::scan, ec);

            // Handle the case where the file doesn't exist
            if (ec == boost::system::errc::no_such_file_or_directory)
                return lambda_(not_found(req_.target()));

            // Handle an unknown error
            if (ec)
                return lambda_(server_error(ec.message()));

            lambda_(file_response(req_.target(), std::move(body)));
        }
        else {
            lambda_(string_response(response.text, response.contentType));
        }
    }

    void onWrite(boost::system::error_code ec, std::size_t bytes_transferred, bool close)
    {
        boost::ignore_unused(bytes_transferred);

        if (ec) {
            return fail(ec, "write");
        }

        if (close) {
            // This means we should close the connection, usually because
            // the response indicated the "Connection: close" semantic.
            return doClose();
        }

        // We're done with the response so delete it
        res_ = nullptr;

        // Read another request
        doRead();
    }

    void doClose()
    {
        // Send a TCP shutdown
        boost::system::error_code ec;
        socket_.shutdown(tcp::socket::shutdown_send, ec);

        // At this point the connection is closed gracefully
    }
};

} // namespace Simple::Http
#endif
