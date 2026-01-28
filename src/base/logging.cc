
#include "logging.h"

#include <iomanip>
#include <sys/time.h>


LogMessage::LogMessage(const char *file, int line, const char *func, int level)
    : file_(file), line_(line), func_(func), level_(level)
{
    Init(file_, line_, func_);
}

LogMessage::~LogMessage()
{
//  auto start = stream_.tellp();

    std::cout << stream_.str();
    stream_ << std::endl;
}

std::ostream& LogMessage::stream()
{
    return stream_;
}

void LogMessage::Init(const char *file, int line, const char *func)
{
    if (level_ < 1)
        return;

    stream_ << '[';

    timeval tv{};
    gettimeofday(&tv, nullptr);
    time_t t = tv.tv_sec;
    struct tm local_time{};
    localtime_r(&t, &local_time);
    struct tm *tm_time = &local_time;
    stream_ << std::setfill('0')
            << std::setw(2) << 1 + tm_time->tm_mon
            << std::setw(2) << tm_time->tm_mday
            << '/'
            << std::setw(2) << tm_time->tm_hour
            << std::setw(2) << tm_time->tm_min
            << std::setw(2) << tm_time->tm_sec
            << '.'
            << std::setw(6) << tv.tv_usec
            << "][";

    std::string file_str;
    file_str.append(file);
    std::string func_str;
    func_str.append(func);
    stream_ << file_str << ":" << line << " (" << func_str << ")]";

    const std::string log_level_str = [&]() -> std::string {
        if (level_ == ERROR) {
            return "[ERROR] ";
        } else if (level_ == WARNING) {
            return "[WARNING] ";
        } else if (level_ == INFO) {
           return "[INFO] ";
        }
        return "[DEBUG] ";
    }();

    stream_ << log_level_str;
}
