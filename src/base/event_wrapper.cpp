#include "event_wrapper.hpp"


#if defined(USE_SELECT)

#include <sys/socket.h>
#include <sys/select.h>
#include <fcntl.h>

#include <vector>
#include <algorithm>

#include "logging.h"


struct Event
{
    Event(SOCKET socket, int type)
        : socket(socket), event_type(type), callback(nullptr)
    {}

    Event(SOCKET socket, int type, Callback cb)
        : socket(socket), event_type(type), callback(std::move(cb))
    {}

    bool operator==(const Event &e) const
    {
        return std::tie(socket, event_type) == std::tie(e.socket, e.event_type);
    }

    bool operator!=(const Event& rhs) const
    {
        return !(*this == rhs);
    }

    SOCKET       socket;
    int          event_type;
    Callback     callback;
};


static int fd_max{-1};
static int loop_fd;
static std::vector<Event> events{};
static bool loop_running{false};


void RunEventLoop()
{
    loop_running = true;
    while (!events.empty())
    {
        fd_set rfds{};
        fd_set wfds{};
        for (const auto & e : events)
        {
            fd_max = std::max(fd_max, e.socket + 1);
            if (e.event_type & kReadEvent)
            {
                FD_SET(e.socket, &rfds);
            }
            if (e.event_type & kWriteEvent)
            {
                FD_SET(e.socket, &wfds);
            }
        }

        struct timeval timeout {2, 500};
        int rv = select(fd_max, &rfds, &wfds, nullptr, &timeout);
        if (rv == -1)
        {
            LOG(ERROR) << "select error: " << errno << " : " << strerror(errno) << std::endl;
            break;
        }

        if (rv > 0)
        {
            for (const auto & e : events)
            {
                if (FD_ISSET(e.socket, &rfds) || FD_ISSET(e.socket, &wfds))
                {
                    e.callback();
                }
            }
        }
    }
}

bool Init()
{
    loop_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (loop_fd < 0)
    {
        LOG(ERROR) << "Init failed: " << errno << " : " << strerror(errno) << std::endl;
        return false;
    }

    fcntl(loop_fd, F_SETFL, fcntl(loop_fd, F_GETFL) | O_NONBLOCK);
    fd_max = std::max(fd_max, loop_fd + 1);

    return true;
}



void EventStart(int socket, int event_type, Callback cb)
{
    Event event{socket, event_type, cb};
    auto iter = std::find(events.begin(), events.end(), event);
    if (iter == events.end())
    {
        fd_max = std::max(socket + 1, fd_max);
        events.emplace_back(event);
    }

    if (!loop_running)
    {
        RunEventLoop();
    }
}


void EventStop(int socket, int event_type)
{
    Event event(socket, event_type);
    auto iter = std::find(events.begin(), events.end(), event);
    if (iter != events.end())
        events.erase(iter);
}


int GetEventQuantityForTest()
{
    return static_cast<int>(events.size());
}

#elif defined(USE_EPOLL)

/*
 * todo : implement
 */

#elif defined(USE_LIBEVENT)

/*
 * todo: implement
 */

#endif