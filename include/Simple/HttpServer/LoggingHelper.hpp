#ifndef SIMPLE_HTTPSERVER_LOGGING_HELPER_HPP
#define SIMPLE_HTTPSERVER_LOGGING_HELPER_HPP

#include <iostream>

#include <boost/system/error_code.hpp>

namespace Simple::Http {

    void fail(boost::system::error_code ec, char const *what)
    {
        std::cerr << what << ": " << ec.message() << "\n";
    }

} // namespace Simple::Http

#endif
