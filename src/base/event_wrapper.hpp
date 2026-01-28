#pragma once

#include <functional>


#if !defined(_WIN32) || !defined(_WIN64)
using SOCKET = int;
#endif


constexpr int kReadEvent = 1 << 1;
constexpr int kWriteEvent = 1 << 2;
constexpr int kTimeoutEvent = 1 << 3;

using Callback = std::function<void()>;

bool Init();
void Release();
void EventStart(SOCKET socket, int event_type, Callback cb);
void EventStop(SOCKET socket, int event_type);

int GetEventQuantityForTest();
