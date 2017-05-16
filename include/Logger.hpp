#ifndef SIMPLE_LOGGER_HPP
#define SIMPLE_LOGGER_HPP

#include <chrono>
#include <ctime>
#include <experimental/filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

enum class LogLevel { INFO, WARNING, ERROR, DEBUG, TRACE };

namespace Simple {

    class Logger {
    public:
        struct Settings {
            Settings() : level(LogLevel::ERROR){};
            LogLevel level;
            std::string filePath;
            std::ofstream file;
            int fileOpenDay;
            std::mutex mtx;
        };

        std::ostringstream &log(LogLevel l = LogLevel::INFO)
        {
            std::time_t t = std::time(nullptr);
            os << std::put_time(std::localtime(&t), "%Y-%m-%d %X");
            os << " @" << std::setw(5) << std::left << std::this_thread::get_id();
            if (l == LogLevel::INFO)
                os << " [INFO] ";
            if (l == LogLevel::WARNING)
                os << " [WARNING] ";
            if (l == LogLevel::ERROR)
                os << " [ERROR] ";
            if (l == LogLevel::DEBUG)
                os << " [DEBUG] ";
            if (l == LogLevel::TRACE)
                os << " [TRACE] ";
            return os;
        };

        ~Logger()
        {
            std::lock_guard<std::mutex> lock(settings.mtx);

            // check if new file should be opened
            checkTimePoint();

            // write log message to stdout or file
            os << '\n';
            if (!settings.file.is_open()) {
                std::cout << os.str();
            }
            else {
                settings.file << os.str();
                settings.file.flush();
            }
        }

        void checkTimePoint()
        {
            namespace fs = std::experimental::filesystem;
            auto currentDay = getCurrentDay();

            if (currentDay == settings.fileOpenDay)
                return;

            std::stringstream ss;
            ss << settings.filePath << "." << settings.fileOpenDay;
            fs::path newPath(ss.str());
            fs::path oldPath(settings.filePath);

            try {
                settings.file.close();
                if (settings.file.fail()) {
                    std::cerr << "Failbit " << settings.file.failbit << std::endl;
                }
                if (fs::exists(newPath))
                    fs::remove(newPath);
                fs::rename(oldPath, newPath);
                settings.file.open(settings.filePath, std::ios::app);
                settings.fileOpenDay = currentDay;
            }
            catch (...) {
                std::cerr << "Cannot remove or rename log file." << '\n';
                std::cerr << "Old path: " << oldPath << '\n';
                std::cerr << "New path: " << newPath << '\n';
            }
        }

        static void setFile(std::string path)
        {
            settings.filePath = path;
            settings.file.open(path, std::ios::app);
            settings.fileOpenDay = getCurrentDay();
        }

        static Settings settings;

    private:
        static int getCurrentDay()
        {
            auto now = std::chrono::system_clock::now();
            auto nowTimeT = std::chrono::system_clock::to_time_t(now);
            return std::localtime(&nowTimeT)->tm_mday;
        }
        std::ostringstream os;
    };

#ifdef INIT_SIMPLE_LOGGER
    Logger::Settings Logger::settings;
#endif

#define LOG(lvl)                                                                                                       \
    if (lvl > Simple::Logger::settings.level)                                                                         \
        ;                                                                                                              \
    else                                                                                                               \
        Simple::Logger().log(lvl)
}

#endif
