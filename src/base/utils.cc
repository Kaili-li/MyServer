#include "utils.h"

#include <cstring>
#include <iostream>


const char CRLF[] = "\r\n\r\n";


bool StringUtils::StartWith(const char* src, const char* needle)
{
    if (src == nullptr || needle == nullptr)
        return false;

    if (strlen(src) < strlen(needle))
        return false;

    bool ok = true;
    for (size_t i = 0; i < strlen(needle); i++)
    {
        if (src[i] != needle[i])
        {
            ok = false;
            break;
        }
    }
    return ok;
}


std::size_t StringUtils::FindCharacterFrom(const char *src, const char c)
{
    return FindCharacterFrom(src, 0, strlen(src), c);
}


std::size_t StringUtils::FindCharacterFrom(const char *src, const std::size_t length, char c)
{
    return FindCharacterFrom(src, 0, length, c);
}


std::size_t StringUtils::FindCharacterFrom(const char *src, std::size_t start_pos, std::size_t end_pos, char c)
{
    if (src == nullptr)
        return -1;

    std::size_t pos = -1;
    for (size_t i = start_pos; i < end_pos; i++)
    {
        if (src[i] == c)
        {
            pos = i;
            break;
        }
    }
    return pos;
}


std::map<std::string, std::string> HttpUtils::ParseHttpHeaderFrom(const char* buf, ssize_t buf_len)
{
    std::map<std::string, std::string> http_header{};

    if (!strstr(buf, CRLF))
        return http_header;

    char http_code[10]{};
    char http_path[500]{};  // TODO: read RFC and get length limit
    char http_method[10]{};
    char http_version[10]{};
    char http_phase[100]{}; // TODO: use correct word

    // Request Header
    if (StringUtils::StartWith(buf, "GET"))
    {
        sscanf(buf, "%s %s HTTP/%s", http_method, http_path, http_version);
        http_header.insert({"http_method", std::string(http_method)});
        http_header.insert({"http_path", std::string(http_path)});
        http_header.insert({"http_version", std::string(http_version)});
    }

    // Response Header
    if (StringUtils::StartWith(buf, "HTTP"))
    {
        sscanf(buf, "HTTP/%s %s %s", http_version, http_code, http_phase);
        http_header.insert({"http_version", std::string(http_version)});
        http_header.insert({"http_code", std::string(http_code)});
        http_header.insert({"http_phase", std::string(http_phase)});
    }

    // get first \r address
    std::size_t addr = StringUtils::FindCharacterFrom(buf, '\r');
    addr += 2;

    // get http header end address
    std::size_t http_header_fin_pos = 0;
    for (std::size_t i = addr; i < static_cast<std::size_t>(buf_len - 1); i++)
    {
        if (buf[i] == '\n' && buf[i+1] == '\r')
        {
            http_header_fin_pos = i;
            break;
        }
    }

    while (addr < http_header_fin_pos)
    {
        size_t colon_pos = StringUtils::FindCharacterFrom(buf, addr, http_header_fin_pos, ':');
        size_t row_end_pos = StringUtils::FindCharacterFrom(buf, addr, http_header_fin_pos, '\r');

        std::string key(buf, addr, colon_pos - addr);
        std::string value(buf, colon_pos + 2, row_end_pos - colon_pos - 2); // exclude colon and space after colon - [: ]
        http_header.insert({key, value});

        addr = row_end_pos + 2;
    }
    return http_header;
}

