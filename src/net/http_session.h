/*
 * HttpSession.hpp
 * Written by likilli on 2020-06-21 14:58
 *
 */


#pragma once


#include <map>
#include <string>

#include "socket_wrapper.hpp"


enum class Status : int
{
    kInit               = 0,
    kRecvRequest        = 1,
    kSendResponseHeader = 2,
    kSendResponseData   = 3
};


class HttpSession
{
public:
    explicit HttpSession(SOCKET socket);
    ~HttpSession();

    HttpSession(HttpSession&&) = delete;
    HttpSession(const HttpSession&) = delete;
    HttpSession& operator=(HttpSession&&) = delete;
    HttpSession& operator=(const HttpSession&) = delete;

public:
    void Start();
    void Read();
    void Send();
    void Close();

private:
    void DoRead(const std::string& data);
    void OnSendDone();
    void OnError(int err_no);

private:
    SocketWrapper socket_{};

    std::string recv_buffer_{};

    Status status_{Status::kInit};
    std::map<std::string, std::string> http_header_{};
};
