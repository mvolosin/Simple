#ifndef SIMPLE_HTTP_CONNECTION_HPP
#define SIMPLE_HTTP_CONNECTION_HPP

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>

#include "Common.hpp"
#include "LoggingHelper.hpp"

namespace Simple::Http {

using tcp = boost::asio::ip::tcp;
namespace websocket = boost::beast::websocket;
namespace http = boost::beast::http;

class Connection : public std::enable_shared_from_this<Connection> {
public:
    enum class State { Connected, Disconnected };

    using Message = std::string;
    using WeakPtr = std::weak_ptr<Connection>;
    using MessageCallback = std::function<void(Connection::WeakPtr, Message)>;
    using StateCallback = std::function<void(Connection::WeakPtr, State)>;
    using Callbacks = std::tuple<MessageCallback, StateCallback>;

    Connection(tcp::socket&& socket, Callbacks callbacks)
        : strand_{socket.get_io_service()}
        , ws_{std::forward<tcp::socket>(socket)}
        , msgCallback_{std::get<0>(callbacks)}
        , stateCallback_{std::get<1>(callbacks)}
    {
    }

    // Start the asynchronous operation
    template <class Body, class Allocator>
    void run(http::request<Body, http::basic_fields<Allocator>> req)
    {
        // Accept the websocket handshake
        ws_.async_accept(req, strand_.wrap(std::bind(&Connection::onAccept, shared_from_this(),
                                                     std::placeholders::_1)));
    }

    virtual void send(std::string&& message) final
    {
        outputBuffer_ = std::move(message);
        ws_.text(ws_.got_text());
        ws_.async_write(boost::asio::buffer(outputBuffer_),
                        strand_.wrap(std::bind(&Connection::onWrite, shared_from_this(),
                                               std::placeholders::_1, std::placeholders::_2)));
    }

private:
    void onAccept(boost::system::error_code ec)
    {
        // Happens when the timer closes the socket
        if (ec == boost::asio::error::operation_aborted) {
            return;
        }

        if (ec) {
            return fail(ec, "accept");
        }

        // read a message
        doRead();

        // notify about connected client
        if (stateCallback_ != nullptr) {
            stateCallback_(weak_from_this(), State::Connected);
        }
    }

    void doRead()
    {
        // Read a message into our buffer
        ws_.async_read(buffer_,
                       strand_.wrap(std::bind(&Connection::onRead, shared_from_this(),
                                              std::placeholders::_1, std::placeholders::_2)));
    }

    void onRead(boost::system::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);
        // Happens when the timer closes the socket
        if (ec == boost::asio::error::operation_aborted) {
            doClose();
            return;
        }

        // This indicates that the websocket_session was closed
        if (ec == websocket::error::closed) {
            doClose();
            return;
        }

        if (ec) {
            doClose();
            fail(ec, "read");
        }

        if (msgCallback_ != nullptr) {
            msgCallback_(weak_from_this(), toString(buffer_));
        }
    }

    void onWrite(boost::system::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        // Happens when the timer closes the socket
        if (ec == boost::asio::error::operation_aborted) {
            doClose();
            return;
        }

        if (ec) {
            doClose();
            return fail(ec, "write");
        }

        // Clear the buffer
        buffer_.consume(buffer_.size());

        // Do another read
        doRead();
    }

    std::string toString(boost::beast::multi_buffer& buffers)
    {
        std::stringstream ss;
        ss << boost::beast::buffers(buffers.data());
        return ss.str();
    }

    void doClose()
    {
        // notify about connected client
        if (stateCallback_ != nullptr) {
            stateCallback_(weak_from_this(), State::Disconnected);
        }
    }

private:
    boost::asio::io_service::strand strand_;
    websocket::stream<tcp::socket> ws_;
    MessageCallback msgCallback_;
    StateCallback stateCallback_;
    boost::beast::multi_buffer buffer_;
    std::string outputBuffer_;
};

using Callbacks =
    std::tuple<RequestCallback, Connection::MessageCallback, Connection::StateCallback>;

} // namespace Simple::Http

#endif
