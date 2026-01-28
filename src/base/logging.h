
#pragma once

// TODO: Move these head importation into cpp file
#include <iostream>
#include <sstream>

constexpr int DEBUG = 1;
constexpr int INFO = 2;
constexpr int WARNING = 3;
constexpr int ERROR = 4;

class LogMessage
{
public:
    LogMessage(const char *file, int line, const char *func, int level);
    ~LogMessage();
    std::ostream& stream();

private:
    void Init(const char *file, int line, const char *func);

private:
    std::ostringstream stream_;

    const char *file_;
    const int line_;
    const char *func_;
    const int level_;
};

#define LOG(condition) LogMessage(__FILE__, __LINE__, __FUNCTION__, condition).stream()
