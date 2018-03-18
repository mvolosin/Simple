#ifndef SIMPLE_HTTPSERVER_WEBSOCKET_SESSION_HPP
#define SIMPLE_HTTPSERVER_WEBSOCKET_SESSION_HPP

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

template <typename Context, typename Handler>
class WebsocketSession : public std::enable_shared_from_this<WebsocketSession<Context, Handler>> {
    boost::asio::io_service::strand strand_;
    websocket::stream<tcp::socket> ws_;
    Context& context_;
    Handler handler_;
    boost::beast::multi_buffer buffer_;
    std::string outputBuffer_;

public:
    WebsocketSession(tcp::socket&& socket, Context& context)
        : strand_{socket.get_io_service()}
        , ws_{std::forward<tcp::socket>(socket)}
        , context_{context}
        , handler_{context}
    {
    }

    // Start the asynchronous operation
    template <class Body, class Allocator>
    void run(http::request<Body, http::basic_fields<Allocator>> req)
    {
        // Accept the websocket handshake
        ws_.async_accept(
            req, strand_.wrap(std::bind(&WebsocketSession::onAccept, this->shared_from_this(), std::placeholders::_1)));
    }

private:
    void onAccept(boost::system::error_code ec)
    {
        // Happens when the timer closes the socket
        if (ec == boost::asio::error::operation_aborted)
            return;

        if (ec)
            return fail(ec, "accept");

        // Read a message
        doRead();
    }

    void doRead()
    {
        // Read a message into our buffer
        ws_.async_read(buffer_, strand_.wrap(std::bind(&WebsocketSession::onRead, this->shared_from_this(),
                                                       std::placeholders::_1, std::placeholders::_2)));
    }

    void onRead(boost::system::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);
        // Happens when the timer closes the socket
        if (ec == boost::asio::error::operation_aborted)
            return;

        // This indicates that the websocket_session was closed
        if (ec == websocket::error::closed)
            return;

        if (ec)
            fail(ec, "read");

        outputBuffer_ = handler_(buffer_);

        // Echo the message
        ws_.text(ws_.got_text());
        ws_.async_write(boost::asio::buffer(outputBuffer_),
                        strand_.wrap(std::bind(&WebsocketSession::onWrite, this->shared_from_this(),
                                               std::placeholders::_1, std::placeholders::_2)));
    }

    void onWrite(boost::system::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        // Happens when the timer closes the socket
        if (ec == boost::asio::error::operation_aborted)
            return;

        if (ec)
            return fail(ec, "write");

        // Clear the buffer
        buffer_.consume(buffer_.size());

        // Do another read
        doRead();
    }
};
} // namespace Simple::HttpServer

#endif
