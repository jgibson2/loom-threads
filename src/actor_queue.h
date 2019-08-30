//
// Created by john on 7/17/19.
//

#ifndef LOOM_ACTOR_QUEUE_H
#define LOOM_ACTOR_QUEUE_H

#include <optional>
#include "../external/concurrentqueue/concurrentqueue.h"
#include <vector>
#include <functional>
#include <sys/epoll.h>

template<typename T, T DEFAULTVALUE = T()>
class ActorQueue {
public:
    ActorQueue() : _queue() {
        _poll_fd = epoll_create1(0);
    }

    bool send(T &&message) {
            return _queue.enqueue(message);
    }

    bool send(const T& message) {
        return _queue.enqueue(message);
    }

    std::optional<T> receive() {
        thread_local T t = DEFAULTVALUE;
        if (_queue.try_dequeue(t)) {
            return std::optional<T>(t);
        }
        return std::nullopt;
    }

    int getPollable() {
        return _poll_fd;
    }

private:
    moodycamel::ConcurrentQueue<T> _queue;
    int _poll_fd;
};

#endif //LOOM_ACTOR_QUEUE_H