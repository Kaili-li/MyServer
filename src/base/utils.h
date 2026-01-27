#pragma once


#include <map>
#include <string>


class StringUtils
{
public:
    static bool StartWith(const char* src, const char* needle);
    static size_t FindCharacterFrom(const char* src, char c);
    static size_t FindCharacterFrom(const char* src, std::size_t length, char c);
    static size_t FindCharacterFrom(const char* src, std::size_t start_pos, std::size_t end_pos, char c);
};


class HttpUtils
{
public:
    static std::map<std::string, std::string> ParseHttpHeaderFrom(const char* buf, ssize_t buf_len);
};