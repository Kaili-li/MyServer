#pragma once


#include <string>
#include <functional>


#if !defined(_WIN32) || !defined(_WIN64)
using SOCKET = int;
#endif


using OnReadCallback = std::function<void()>;
using OnDataCallback = std::function<void(std::string& data)>;
using OnDoneCallback = std::function<void()>;
using OnErrorCallback = std::function<void(int)>;


class SocketWrapper
{
public:
    explicit SocketWrapper(bool IPv6 = false);
    explicit SocketWrapper(SOCKET socket);
    ~SocketWrapper();

public:
    void ToNonBlockMode();
    SOCKET GetSocket() const;

    void StartRead();
    void StopRead() const;
    void Send(const std::string& data);
    void Send(const char* data, size_t data_size);
    void Close();

    void SetOnReadCallback(OnReadCallback cb);
    void SetOnDataCallback(OnDataCallback cb);
    void SetOnDoneCallback(OnDoneCallback cb);
    void SetOnErrorCallback(OnErrorCallback cb);    

private:
    void DoSend();
    void DoRead();

private:
    SOCKET socket_{};

    std::size_t sent_len_{};
    // std::size_t recv_len_{};
    std::string send_buffer_{};
    std::string recv_buffer_{};

    OnReadCallback on_read_{};
    OnDataCallback on_data_{};
    OnDoneCallback on_done_{};
    OnErrorCallback on_error_{};
};
