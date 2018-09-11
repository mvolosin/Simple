#include <Simple/HttpServer/Server.hpp>

namespace Simple::Http {

Server::Listener::Listener(boost::asio::io_context& ios, tcp::endpoint endpoint)
    : strand_{ios}
    , acceptor_{ios}
    , socket_{ios}
{
    boost::system::error_code ec;

    // Open the acceptor
    acceptor_.open(endpoint.protocol(), ec);
    if (ec) {
        fail(ec, "open");
        return;
    }
    acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));

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

void Server::Listener::setCallbacks(Callbacks callbacks)
{
    callbacks_ = std::move(callbacks);
}

void Server::Listener::doAccept()
{
    acceptor_.async_accept(socket_, strand_.wrap(std::bind(&Listener::onAccept, this, std::placeholders::_1)));
}

void Server::Listener::onAccept(boost::system::error_code ec)
{
    if (ec) {
        fail(ec, "accept");
        return;
    }
    else {
        // Create the HTTP session and start listening
        std::make_shared<Session>(std::move(socket_), callbacks_)->run();
    }

    // Accept another connection
    doAccept();
}

Server::Server(const std::string& address, unsigned short port, int threads)
    : threads_{threads == 0 ? std::thread::hardware_concurrency() : threads}
    , ioc_{threads}
    , listener_{ioc_, tcp::endpoint{boost::asio::ip::address::from_string(address), port}}
{
}

void Server::start()
{
    listener_.setCallbacks(callbacks_);

    boost::asio::signal_set signals{ioc_, SIGINT, SIGTERM};
    std::function<void(const boost::system::error_code&, int)> sigHandler;
    sigHandler = [&, this](const boost::system::error_code& error, int signal) {
        boost::ignore_unused(signal);
        if (!error) {
            ioc_.stop();
        }
        signals.async_wait(sigHandler);
    };
    signals.async_wait(sigHandler);

    std::vector<std::thread> ths;
    ths.reserve(threads_ - 1);
    for (auto i = threads_ - 1; i > 0; --i) {
        ths.emplace_back([&]() { ioc_.run(); });
    }
    ioc_.run();
    for (auto& t : ths) {
        t.join();
    }
}

void Server::onRequest(RequestCallback callback)
{
    std::get<0>(callbacks_) = std::move(callback);
}

void Server::onMessage(Connection::MessageCallback callback)
{
    std::get<1>(callbacks_) = std::move(callback);
}

void Server::onConnection(Connection::StateCallback callback)
{
    std::get<2>(callbacks_) = std::move(callback);
}
} // namespace Simple::Http
