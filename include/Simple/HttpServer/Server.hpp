#ifndef SIMPLE_HTTP_SERVER_HPP
#define SIMPLE_HTTP_SERVER_HPP

#include <memory>
#include <thread>
#include <vector>

#include <boost/asio.hpp>
#include <boost/asio/signal_set.hpp>

#include "LoggingHelper.hpp"
#include "Session.hpp"

namespace Simple::Http {

class Server {
    class Listener {
        boost::asio::io_service::strand strand_;
        tcp::acceptor acceptor_;
        tcp::socket socket_;
        Callbacks callbacks_;

    public:
        explicit Listener(boost::asio::io_context& ios, tcp::endpoint endpoint);
        void setCallbacks(Callbacks callbacks);

    private:
        void doAccept();
        void onAccept(boost::system::error_code ec);
    };

    size_t threads_;
    boost::asio::io_context ioc_;
    Listener listener_;
    Callbacks callbacks_;

public:
    explicit Server(const std::string& address, unsigned short port, int threads = 0);
    void start();
    void onRequest(RequestCallback callback);
    void onMessage(Connection::MessageCallback callback);
    void onConnection(Connection::StateCallback callback);
};
} // namespace Simple::Http

#endif
