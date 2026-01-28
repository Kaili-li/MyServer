#include "socket_wrapper.hpp"

#include <unistd.h>
#include <sys/socket.h>
#include <utility>
#include <fcntl.h>
#include <cerrno>
#include <cassert>

#include "event_wrapper.hpp"


constexpr int kBufSize = 8192;


SocketWrapper::SocketWrapper(bool IPv6)
{
    if (!IPv6)
        socket_ = socket(PF_INET, SOCK_STREAM, 0);
    else
        socket_ = socket(PF_INET6, SOCK_STREAM, 0);

    fcntl(socket_, F_SETFL, fcntl(socket_, F_GETFL) | O_NONBLOCK);
}


SocketWrapper::SocketWrapper(SOCKET fd) : socket_(fd)
{
    fcntl(socket_, F_SETFL, fcntl(socket_, F_GETFL) | O_NONBLOCK);
}


SocketWrapper::~SocketWrapper()
{
    Close();    // TODO: Charify here: DO NOT Call any function in destuctor (Effective C++);
}


SocketWrapper::SocketWrapper(SocketWrapper &&ps) noexcept
{
    socket_ = ps.socket_;

    sent_len_ = ps.sent_len_;
    recv_len_ = ps.recv_len_;
    send_buffer_ = std::move(ps.send_buffer_);
    recv_buffer_ = std::move(ps.recv_buffer_);

    on_read_  = std::move(ps.on_read_);
    on_data_  = std::move(ps.on_data_);
    on_done_  = std::move(ps.on_done_);
    on_error_ = std::move(ps.on_error_);


    ps.socket_ = -1;
    ps.sent_len_ = -1;
    ps.recv_len_ = -1;
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


void SocketWrapper::StartRead()
{
    if (on_read_)
        EventStart(socket_, kReadEvent, on_read_);
    else
        EventStart(socket_, kReadEvent, [&](){ DoRead(); });
}


void SocketWrapper::StopRead() const
{
    EventStop(socket_, kReadEvent);
}


void SocketWrapper::SetOnReadCallback(OnReadCallback cb)
{
    on_read_ = std::move(cb);
}


void SocketWrapper::SetOnDataCallback(OnDataCallback cb)
{
    on_data_ = std::move(cb);
}


void SocketWrapper::StartSend()
{
    EventStart(socket_, kWriteEvent, [&](){ DoSend(); });
}


void SocketWrapper::StopSend() const
{
    EventStop(socket_, kWriteEvent);
}


void SocketWrapper::SetSendData(const std::string& data)
{
    send_buffer_.append(data);
}


void SocketWrapper::SetSendData(const char *data, size_t data_size)
{
    send_buffer_.append(data, data_size);
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
    ssize_t send_len = send(socket_, send_buffer_.c_str() + sent_len_, send_buffer_.length(), 0);
    if (send_len == -1)
    {
#ifdef __APPLE__
        if (errno == EAGAIN)
#elif __linux__
        if (errno == EAGAIN || errno == EWOULDBLOCK)
#endif
            return;
        else
            on_error_(errno);
    }

    sent_len_ += send_len;
    if (sent_len_ == send_buffer_.length())     //  TODO: Optimize here when send buffer is very large (huge memory use)
        on_done_();
}


void SocketWrapper::DoRead()
{
    char buf[kBufSize]{};
    ssize_t recv_len = recv(socket_, buf, kBufSize - 1, 0);
    if (recv_len == -1)
    {
#ifdef __APPLE__
        if (errno == EAGAIN)
#elif __linux__
        if (errno == EAGAIN || errno == EWOULDBLOCK)
#endif
            return;
        else
            on_error_(errno);
    }
    if (recv_len > 0)
    {
        recv_buffer_.append(buf, recv_len);
        on_data_(recv_buffer_);
    }
}
