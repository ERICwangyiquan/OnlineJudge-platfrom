#pragma once

#include <iostream>
#include <string>
#include "util.hpp"

namespace ns_log
{
    using namespace ns_util;

    // log level
    enum
    {
        INFO,
        DEBUG,
        WARNING,
        ERROR,
        FATAL
    };

    // LOG() << "message"
    inline std::ostream &Log(const std::string &level, const std::string &file_name, int line)
    {
        // add log level
        std::string message = "[";
        message += level;
        message += "]";

        // add file name
        message += "[";
        message += file_name;
        message += "]";

        // add line num
        message += "[";
        message += std::to_string(line);
        message += "]";

        // add current time-stamp
        message += "[";
        message += TimeUtil::GetTimeStamp();
        message += "]";

        // there's buffer inside of cout
        std::cout << message;   //no endl

        return std::cout;
    }

    #define LOG(level) Log(#level, __FILE__, __LINE__)
}