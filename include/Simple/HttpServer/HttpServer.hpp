#ifndef SIMPLE_HTTPSERVER_HPP
#define SIMPLE_HTTPSERVER_HPP

#include <memory>
#include <thread>
#include <vector>

#include <boost/asio.hpp>

#include "LoggingHelper.hpp"

namespace Simple::HttpServer {

using tcp = boost::asio::ip::tcp;

/// @brief Listener object listen to new clients. If client is connected new http session is
/// created.
template <typename Context, typename HttpSession>
class Listener {
    boost::asio::io_service::strand strand_;
    tcp::acceptor acceptor_;
    tcp::socket socket_;
    std::string const& docRoot_;
    Context context;

public:
    explicit Listener(boost::asio::io_service& ios, tcp::endpoint endpoint, std::string const& docRoot)
        : strand_{ios}
        , acceptor_{ios}
        , socket_{ios}
        , docRoot_{docRoot}
    {
        boost::system::error_code ec;

        // Open the acceptor
        acceptor_.open(endpoint.protocol(), ec);
        if (ec) {
            fail(ec, "open");
            return;
        }

        // Bind to the server address
        acceptor_.bind(endpoint, ec);
        if (ec) {
            fail(ec, "bind");
            return;
        }

        // Start listening for connections
        acceptor_.listen(boost::asio::socket_base::max_connections, ec);
        if (ec) {
            fail(ec, "listen");
            return;
        }

        // post accept job
        doAccept();
    }

private:
    void doAccept()
    {
        acceptor_.async_accept(socket_, std::bind(&Listener::onAccept, this, std::placeholders::_1));
    }

    void onAccept(boost::system::error_code ec)
    {
        if (ec) {
            fail(ec, "accept");
        }
        else {
            // Create the HTTP session and start listening
            std::make_shared<HttpSession>(std::move(socket_), docRoot_, context)->run();
        }

        // Accept another connection
        doAccept();
    }
};

template <typename Context, typename HttpSession>
class HttpServer {
    size_t threads_;
    boost::asio::io_service ios_;
    Listener<Context, HttpSession> listener_;

public:
    explicit HttpServer(std::string& address, unsigned short port, const std::string& docRoot, size_t threads)
        : threads_{threads}
        , ios_(threads)
        , listener_(ios_, tcp::endpoint{boost::asio::ip::address::from_string(address), port}, docRoot)
    {
    }

    void start()
    {
        std::vector<std::thread> ths;
        ths.reserve(threads_ - 1);
        for (auto i = threads_ - 1; i > 0; --i) {
            ths.emplace_back([&]() { ios_.run(); });
        }
        ios_.run();
        for (auto& t : ths) {
            t.join();
        }
    }
};
} // namespace Simple::HttpServer

#endif
