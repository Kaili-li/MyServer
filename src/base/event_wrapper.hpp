#pragma once

#include <functional>


#if !defined(_WIN32) || !defined(_WIN64)
using SOCKET = int;
#endif


constexpr int kRead = 1 << 1;
constexpr int kWrite = 1 << 2;

using Callback = std::function<void()>;

struct Event
{
    Event(SOCKET socket, int type, Callback cb);
    bool operator==(const Event &e) const;

    SOCKET       socket;
    int          event_type;
    Callback     callback;
};

void EventAdd(SOCKET socket, int event_type, Callback cb);
void EventDel(SOCKET socket, int event_type);
void StartEventLoop();

int GetEventQuantityForTest();
