#include "../include/Logger.hpp"

int main()
{
    LOG_REPORTING_LEVEL("Trace");
    LOG_INFO << "Information log line";
    LOG_WARNING << "Warning log line";
    LOG_ERROR << "Error log line";
    LOG_DEBUG << "Debug log line";
    LOG_TRACE << "Trace log line";
    return 0;
}
