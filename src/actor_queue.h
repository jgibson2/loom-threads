//
// Created by john on 7/17/19.
//

#ifndef LOOM_ACTOR_QUEUE_H
#define LOOM_ACTOR_QUEUE_H

#include <optional>
#include <vector>
#include <functional>
#include <sys/epoll.h>
#include <iostream>
#include <typeindex>
#include "../external/concurrentqueue/concurrentqueue.h"

class ActorQueueBase {
public:
    virtual ~ActorQueueBase() = default;
};

template<typename T, T DEFAULTVALUE = T()>
class ActorQueue : public ActorQueueBase {
public:
    const std::type_index typeIndex = std::type_index(typeid(T));

    ActorQueue() : _queue() {
        _poll_fd = epoll_create1(0);
    }

    ~ActorQueue() override = default;

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

    std::vector<T>& bulkReceive(size_t max_size = 1024) {
        thread_local std::vector<T> _receive_vec; //reuse mem
        auto numElems = std::min(_queue.size_approx(), max_size); //max limit of elements to receive
        _receive_vec.reserve(numElems); //reserve at least that much mem
        _receive_vec.clear();
        numElems = _queue.try_dequeue_bulk(_receive_vec.begin(), numElems); //true number of elements received
        _receive_vec.resize(numElems); //resize to correct size
        return _receive_vec;
    }

    int getPollable() {
        return _poll_fd;
    }

private:
    moodycamel::ConcurrentQueue<T> _queue;
    int _poll_fd;
};

#endif //LOOM_ACTOR_QUEUE_H
