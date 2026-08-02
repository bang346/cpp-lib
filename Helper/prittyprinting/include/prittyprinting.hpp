#ifndef PRITTY_PRINTING_HPP
#define PRITTY_PRINTING_HPP

#include <chrono>
#include <cstdio>
#include <ctime>
#include <string>

// the following are UBUNTU/LINUX, and MacOS ONLY terminal color codes.
#define RESET "\033[0m"
#define BLACK "\033[30m"              /* Black */
#define RED "\033[31m"                /* Red */
#define GREEN "\033[32m"              /* Green */
#define YELLOW "\033[33m"             /* Yellow */
#define BLUE "\033[34m"               /* Blue */
#define MAGENTA "\033[35m"            /* Magenta */
#define CYAN "\033[36m"               /* Cyan */
#define WHITE "\033[37m"              /* White */
#define BOLDBLACK "\033[1m\033[30m"   /* Bold Black */
#define BOLDRED "\033[1m\033[31m"     /* Bold Red */
#define BOLDGREEN "\033[1m\033[32m"   /* Bold Green */
#define BOLDYELLOW "\033[1m\033[33m"  /* Bold Yellow */
#define BOLDBLUE "\033[1m\033[34m"    /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m" /* Bold Magenta */
#define BOLDCYAN "\033[1m\033[36m"    /* Bold Cyan */
#define BOLDWHITE "\033[1m\033[37m"   /* Bold White */

// from: https://stackoverflow.com/questions/9158150/colored-output-in-c/9158263

std::string get_current_time()
{
    using namespace std::chrono;

    const auto now = system_clock::now();
    const std::time_t now_time = system_clock::to_time_t(now);

    std::tm local_time{};

    // Windows-sichere Variante von localtime()
    if (localtime_s(&local_time, &now_time) != 0)
    {
        return {};
    }

    const auto milliseconds =
        duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) %
        1000;

    char buffer[16]{};

    std::snprintf(
        buffer,
        sizeof(buffer),
        "%02d:%02d:%02d.%03lld",
        local_time.tm_hour,
        local_time.tm_min,
        local_time.tm_sec,
        static_cast<long long>(milliseconds.count()));

    return std::string{buffer};
}

#endif