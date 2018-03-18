#ifndef SIMPLE_HTTPSERVER_WEBSOCKETDATAHANDLER_HPP
#define SIMPLE_HTTPSERVER_WEBSOCKETDATAHANDLER_HPP

#include <boost/beast.hpp>
#include <boost/beast/websocket.hpp>

#include "MimeType.hpp"

namespace Simple::HttpServer {

namespace http = boost::beast::http;

class IWebsocketDataHandler {
public:
    virtual ~IWebsocketDataHandler() = default;
    virtual std::string operator()(boost::beast::multi_buffer& buffer) = 0;
};

template <class ConstBufferSequence>
std::string to_string(ConstBufferSequence const& buffers)
{
    std::string s;
    s.reserve(boost::asio::buffer_size(buffers));
    for (boost::asio::const_buffer b : buffers)
        s.append(boost::asio::buffer_cast<char const*>(b), boost::asio::buffer_size(b));
    return s;
}

template <class Context>
class WebsocketDataHandler : public IWebsocketDataHandler {
public:
    WebsocketDataHandler(Context& context)
        : context_{context}
    {
    }

    std::string operator()(boost::beast::multi_buffer& buffer)
    {
        return to_string(buffer.data());
    }

private:
    Context& context_;
};
} // namespace Simple::HttpServer

#endif
