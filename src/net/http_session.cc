/*
 * HttpSession.cpp
 * Written by likai on 2020-06-21 14:59
 *
 */


#include <cstring>

#include "http_session.h"
#include "utils.h"
#include "logging.h"


const char* kHeader = "HTTP/1.1 200 OK\r\n"
                      "Connection: close\r\n"
                      "Content-Type: text/html; charset=UTF-8\r\n"
                      "Content-Length: 85\r\n"
                      "Server:elitk/Manjaro 20.0 - Lysia\r\n"
                      "\r\n"
                      "<!DOCTYPE html><html><head> Welcom, Kai</head><h1> Hi, Home page by Kai Li</h1></html>";


HttpSession::HttpSession(SOCKET socket) : socket_(socket)
{
    socket_.ToNonBlockMode();
}

HttpSession::~HttpSession()
{
    Close();
}


void HttpSession::Start()
{
    Read();
}


void HttpSession::Read()
{
    socket_.SetOnDataCallback([&](std::string& data){ DoRead(data); });
    socket_.SetOnErrorCallback([&](int err_no){ OnError(err_no); });
    socket_.StartRead();
}


void HttpSession::Send()
{
    socket_.SetOnDoneCallback([&](){ OnSendDone(); });
    socket_.SetOnErrorCallback([&](int err_no){ OnError(err_no); });
    socket_.Send(kHeader, strlen(kHeader));
}


void HttpSession::Close()
{
    if (!http_header_.empty())
        http_header_.clear();

    if (!recv_buffer_.empty())
        recv_buffer_.clear();

    socket_.Close();
}


void HttpSession::DoRead(std::string& data)
{
    recv_buffer_.append(data);
    data.clear(); // TODO: Optimize here

    http_header_ = HttpUtils::ParseHttpHeaderFrom(recv_buffer_.c_str(), recv_buffer_.length());
    if (!http_header_.empty())
    {
        socket_.StopRead();
        LOG(INFO) << "Request Header: " << std::endl;
        for (const auto& t : http_header_)
            LOG(DEBUG) << t.first << " : " << t.second << std::endl;

        status_ = Status::kSendResponseHeader;
        Send();
    }
}


void HttpSession::OnSendDone()
{
    Close();
}


void HttpSession::OnError(int err_no)
{
    LOG(ERROR) << "errno: " << err_no << " : " << strerror(err_no) << std::endl;
    switch (status_)
    {
        case Status::kInit:
            break;
        case Status::kRecvRequest:
            socket_.StopRead();
        case Status::kSendResponseHeader:
        case Status::kSendResponseData:
            break;
    }
    Close();
}
