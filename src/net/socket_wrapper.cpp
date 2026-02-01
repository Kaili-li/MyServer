#include "socket_wrapper.hpp"

#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <utility>
#include <fcntl.h>
#include <cerrno>
#include <cassert>

#include "event_wrapper.hpp"
#include "logging.h"


constexpr int kBufSize = 8192;


SocketWrapper::SocketWrapper(bool IPv6)
{
    if (!IPv6)
        socket_ = socket(PF_INET, SOCK_STREAM, 0);
    else
        socket_ = socket(PF_INET6, SOCK_STREAM, 0);
}

SocketWrapper::SocketWrapper(SOCKET socket) : socket_(socket)
{}


SocketWrapper::~SocketWrapper()
{
    Close();    // TODO: Charify here: DO NOT Call any function in destuctor (Effective C++);
}

void SocketWrapper::ToNonBlockMode() const
{
     fcntl(socket_, F_SETFL, fcntl(socket_, F_GETFL) | O_NONBLOCK);;
}

SOCKET SocketWrapper::GetSocket() const 
{
    return socket_; 
}

void SocketWrapper::Close()
{
    if (socket_ != -1)
    {
        shutdown(socket_, SHUT_RDWR);
        EventStop(socket_, kReadEvent);
        EventStop(socket_, kWriteEvent);
        close(socket_);
        socket_ = -1;
    }
}


void SocketWrapper::Read()
{
    DoRead();
}


void SocketWrapper::StopRead() const
{
    EventStop(socket_, kReadEvent);
}

void SocketWrapper::Send(const std::string& data)
{
    send_buffer_.append(data);
    DoSend();
}

void SocketWrapper::Send(const char *data, size_t data_size)
{
    send_buffer_.append(data, data_size);
    DoSend();
}


void SocketWrapper::SetOnReadCallback(OnReadCallback cb)
{
    assert(cb != nullptr);
    on_read_ = std::move(cb);
}


void SocketWrapper::SetOnDataCallback(OnDataCallback cb)
{
    assert(cb != nullptr);
    on_data_ = std::move(cb);
}


void SocketWrapper::SetOnDoneCallback(OnDoneCallback cb)
{
    assert(cb != nullptr);
    on_done_ = std::move(cb);
}


void SocketWrapper::SetOnErrorCallback(OnErrorCallback cb)
{
    assert(cb != nullptr);
    on_error_ = std::move(cb);
}


void SocketWrapper::DoSend()
{
    const ssize_t send_len = send(socket_, send_buffer_.c_str(), send_buffer_.length(), 0);
    if (send_len == -1)
    {
#ifdef __APPLE__
        if (errno == EAGAIN) {
#elif __linux__
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
#else 
        // TODO : Support Windows
#endif
            EventStart(socket_, kWriteEvent, [&](){ DoSend(); });
        }
        else
        {
            LOG(ERROR) << "Send data failed: " << errno << ", " << strerror(errno) << std::endl;
            on_error_(errno);
        }

        return;
    }

    send_buffer_.erase(send_buffer_.begin(), send_buffer_.begin() + send_len);
    if (send_buffer_.empty())
    {
        EventStop(socket_, kWriteEvent);
        on_done_();
    }
}


void SocketWrapper::DoRead()
{
    char buf[kBufSize]{};
    const ssize_t recv_len = recv(socket_, buf, kBufSize - 1, 0);
    if (recv_len == -1)
    {
#ifdef __APPLE__
        if (errno == EAGAIN) 
#elif __linux__
        if (errno == EAGAIN || errno == EWOULDBLOCK)
#else 
        // TODO: Support Windows
#endif
        {
            EventStart(socket_, kReadEvent, [&](){ DoRead(); });
        }
        else
        {
            LOG(ERROR) << "Socket recv failed, errno: " << errno << " : " << strerror(errno) << std::endl;
            on_error_(errno);
        }
    }
    else if (recv_len == 0)
    {
        EventStop(socket_, kReadEvent);
    }
    else
    {
        std::string data(buf, recv_len);
        on_data_(data);
    }
}
