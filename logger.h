#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <iomanip>
#include <chrono>

#include "config.h"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_RESET         "\x1b[0m"
#define ANSI_WEIGHT_BOLD   "\e[1m"

#define LOG_FN(NAME, TAG, COLOR, MINLEVEL)      \
    template<typename... Args>                  \
    void NAME(const char *format, Args... args) \
    {                                           \
        if(level < MINLEVEL) return;            \
        log(COLOR, TAG, format, args...);       \
    }

static std::map<std::string, int> LogLevel = {
    {"SILENT",  1},
    {"ERROR",   2},
    {"WARN",    3},
    {"INFO",    4},
    {"DEBUG",   5}
};

class Logger
{
public:
    Logger(Config &config)
    {
        std::string logLevel = config.getLogLevel();
        for (auto & c: logLevel) c = toupper(c);

        if (LogLevel.find(logLevel) == LogLevel.end())
        {
            throw ConfigError("Invalid log level: " + logLevel);
        }

        timestamp = config.getLogTimestamp();
        fancy = config.getLogFancy();
        level = LogLevel[logLevel];
    }

    void log(std::string color, std::string tag, const char *format, ...)
    {
        using std::chrono::system_clock;

        va_list argptr;
        va_start(argptr, format);

        // Print the tag in color and bold
        fancy && std::cout << color << ANSI_WEIGHT_BOLD;
        std::cout << std::left << std::setw(10) << tag;
        std::cout << ANSI_RESET;

        // Print timestamp
        if (timestamp)
        {
            std::time_t t = std::time(nullptr);
            std::tm tm = *std::localtime(&t);
            std::cout << std::left << std::setw(10)
                      << std::put_time(&tm, "%D %T ");
            std::cout << ANSI_RESET;
        }

        // Format the rest of the arguments
        std::vprintf(format, argptr);
        std::cout << std::endl;
        va_end(argptr);
    }

    LOG_FN(error,   "Error",   ANSI_COLOR_RED,     LogLevel["ERROR"])
    LOG_FN(info,    "Info",    ANSI_COLOR_GREEN,   LogLevel["INFO"])
    LOG_FN(warn,    "Warn",    ANSI_COLOR_YELLOW,  LogLevel["WARN"])
    LOG_FN(debug,   "Debug",   ANSI_COLOR_BLUE,    LogLevel["DEBUG"])

    private:
        int level;
        bool fancy;
        bool timestamp;
};
#endif // LOGGER_H
