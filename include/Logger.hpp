#ifndef SIMPLE_LOGGER_HPP
#define SIMPLE_LOGGER_HPP

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
// Windows
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#if _MSC_VER >= 1700
#include <mutex>
#else
#include <boost/thread.hpp>
#endif

#if _MSC_VER >= 1900
#include <filesystem>
#else
#include <boost/filesystem.hpp>
#endif

#else
// Linux and friends
#include <sys/time.h>
#if __cplusplus > 201402L
#include <mutex>
#include <experimental/filesystem>
#else
#include <boost/thread.hpp>
#include <boost/filesystem.hpp>
#endif
#endif

#include <array>
#include <sstream>
#include <string>
#include <iomanip>
#include <iostream>
#include <cstdio>
#include <ctime>
#include <fstream>

namespace Simple {

#if _MSC_VER >= 1700 || __cplusplus > 201402L
    using MutexType = std::mutex;
    using LockType = std::lock_guard<MutexType>;
#else
    typedef boost::mutex MutexType;
    typedef boost::lock_guard<boost::mutex> LockType;
#endif

#if _MSC_VER >= 1900 || __cplusplus > 201402L
    namespace fs = std::experimental::filesystem;
#else
    namespace fs = boost::filesystem;
#endif

    struct LogLevel {
        enum { Error, Warning, Info, Debug, Trace };
    };

    inline std::string NowTime();

    class Logger {
    public:
        static Logger* getInstance()
        {
            static Logger* instance = new Logger();
            return instance;
        };

        static int levelFromString(std::string level) {
            if (level == "Debug")
                return LogLevel::Debug;
            if (level == "Warning")
                return LogLevel::Warning;
            if (level == "Trace")
                return LogLevel::Trace;
            if (level == "Info")
                return LogLevel::Info;
            if (level == "Error")
                return LogLevel::Error;
            return LogLevel::Info;
        }

        void writeStream(std::string&& logLine)
        {
            LockType lock(mtx);
            if (logToFile) {
                if(openTimeIsDifferent())
                    openFile();
                file << logLine << std::endl;
            }
            else {
                std::cerr << logLine << std::endl;
            }
        }

        int& reportingLevel()
        {
            return reportingLevel_;
        }

        void setFileName(std::string dir, std::string name)
        {
            LockType lock(mtx);
            logsDir = dir;
            logsName = name;
            logToFile = true;
            openFile();
        }

    private:
        Logger()
            : reportingLevel_(LogLevel::Info)
            , logToFile(false)
        {};
        Logger(Logger const&) {};
        void operator=(Logger const&) {};

        bool openTimeIsDifferent()
        {
            const auto t = std::time(nullptr);
            tm now;
#ifdef WIN32
            localtime_s(&now, &t);
#else
            now = *std::localtime(&t);
#endif
            tm last;
#ifdef WIN32
            localtime_s(&last, &lastOpenTime);
#else
            last = *std::localtime(&lastOpenTime);
#endif

            bool dateDiffer = (now.tm_year != last.tm_year
                || now.tm_mon != last.tm_mon
                || now.tm_mday != last.tm_mday);
            return dateDiffer;
        }

        void openFile()
        {
            if (file.is_open()) {
                file.close();
            }

            lastOpenTime = std::time(nullptr);
            tm td;
#ifdef WIN32
            localtime_s(&td, &lastOpenTime);
#else
            td = *std::localtime(&lastOpenTime);
#endif
            std::stringstream ss;
            if (!logsDir.empty()) {
                try {
                    if (!fs::exists(logsDir))
                        fs::create_directories(logsDir);
                }
                catch (...) {
                    std::cerr << "Cannot create directory: " << logsDir << std::endl;
                }
                ss << logsDir << "/";
            }
            ss  << logsName
                << "_" << (td.tm_year + 1900)
                << "-" << std::setw(2) << std::setfill('0') << td.tm_mon + 1
                << "-" << std::setw(2) << std::setfill('0') << td.tm_mday
                << ".log";
            std::cerr << "Opening log file: " << ss.str() << std::endl;
            file.open(ss.str(), std::ios::app);
        }

        int reportingLevel_;
        bool logToFile;
        time_t lastOpenTime;
        MutexType mtx;
        std::string logsName;
        std::string logsDir;
        std::ofstream file;
    };

    class LogLine {
    public:
        explicit LogLine(int level)
        {
            os_ << NowTime();
            os_ << " " << std::setw(9) << levelToString(level) + ": ";
        }

        ~LogLine()
        {
            Simple::Logger::getInstance()->writeStream(os_.str());
        }

        std::ostringstream& stream() { return os_; }

    private:
        const std::string& levelToString(int level)
        {
            static const std::array<std::string, 5> levels = {"ERROR", "WARNING", "INFO", "DEBUG", "TRACE"};
            return levels[level];
        }

        std::ostringstream os_;
    };

    template <typename T>
    LogLine& operator << (LogLine&& ll, T && arg)
    {
        ll.stream() << std::forward<T>(arg);
        return ll;
    }

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
    inline std::string NowTime()
    {
        const int MAX_LEN = 200;
        char buffer[MAX_LEN];
        if (GetTimeFormatA(LOCALE_USER_DEFAULT, 0, 0,
            "HH':'mm':'ss", buffer, MAX_LEN) == 0)
            return "Error in NowTime()";

        char dateBuffer[MAX_LEN];
        if (GetDateFormatA(LOCALE_USER_DEFAULT, 0, 0,
            "dd'.'MM'.'yyyy", dateBuffer, MAX_LEN) == 0)
            return "Error in NowTime()";

        char result[100] = { 0 };
        static DWORD first = GetTickCount();
        sprintf_s(result, "%s %s.%03ld", dateBuffer, buffer, (long)(GetTickCount() - first) % 1000);
        return result;
    }
#else
    inline std::string NowTime()
    {
        char buffer[11];
        time_t t;
        time(&t);
        tm r = { 0 };
        strftime(buffer, sizeof(buffer), "%X", localtime_r(&t, &r));
        struct timeval tv;
        gettimeofday(&tv, 0);
        char result[100] = { 0 };
        std::sprintf(result, "%s.%03ld", buffer, (long)tv.tv_usec / 1000);
        return result;
    }
#endif //WIN32

} //namespace Simple

#define LOG(level) \
    if (level > Simple::Logger::getInstance()->reportingLevel()) ;\
    else Simple::LogLine(level)

#define LOG_INFO \
    LOG(Simple::LogLevel::Info)

#define LOG_WARNING \
    LOG(Simple::LogLevel::Warning)

#define LOG_DEBUG \
    LOG(Simple::LogLevel::Debug)

#define LOG_TRACE \
    LOG(Simple::LogLevel::Trace)

#define LOG_ERROR \
    LOG(Simple::LogLevel::Error)

#define LOG_REPORTING_LEVEL(level) \
    Simple::Logger::getInstance()->reportingLevel() = Simple::Logger::levelFromString(level)

#define LOG_TO_FILE(dir, file) \
    Simple::Logger::getInstance()->setFileName(dir, file)

#endif // SIMPLE_LOGGER_HPP
