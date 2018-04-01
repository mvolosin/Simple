#include <iostream>

#include <Simple/HttpServer/Server.hpp>

using Simple::Http::Connection;
using Simple::Http::Response;
using Simple::Http::StringRequest;

int main()
{
    std::string webroot = "www/";
    try {
        Simple::Http::Server server{"127.0.0.1", 9999};

        server.onRequest([&](const StringRequest& req) {
            std::cout << "Request: " << req.method_string() << ' ' << req.target() << std::endl;
            Response r;
            r.file = webroot + req.target().to_string();
            r.type = Response::Type::File;
            return r;
        });

        server.onConnection([](Connection::WeakPtr connection, Connection::State state) {
            if (state == Connection::State::Connected) {
                std::cout << "WS Client: " << connection.lock().get() << " connected." << std::endl;
            }
            else {
                std::cout << "WS Client: " << connection.lock().get() << " disconnected."
                          << std::endl;
            }
        });

        server.onMessage([](Connection::WeakPtr connection, Connection::Message message) {
            std::cout << "Client " << connection.lock().get() << " says: " << message << std::endl;
            return message;
        });

        server.start();
    }
    catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
    }

    return 0;
}
