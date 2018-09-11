#ifndef SIMPLE_HTTP_COMMON_HPP
#define SIMPLE_HTTP_COMMON_HPP

#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <functional>

namespace Simple::Http {

namespace http = boost::beast::http;

using StringRequest = http::request<http::string_body>;

using StringResponse = http::response<http::string_body>;
using FileResponse = http::response<http::file_body>;
using EmptyResponse = http::response<http::empty_body>;

struct Response {
    enum class Type { String, File };
    std::string text;
    std::string file;
    std::string contentType{"text/html"};
    Type type{Type::File};
};

using RequestCallback = std::function<Response(const StringRequest&)>;
} // namespace Simple::Http

#endif
