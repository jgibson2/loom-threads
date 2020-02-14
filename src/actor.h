//
// Created by john on 7/17/19.
//

#ifndef LOOM_ACTOR_H
#define LOOM_ACTOR_H

#include "../external/concurrentqueue/concurrentqueue.h"
#include <optional>
#include <functional>
#include "uuid.h"
#include "actor_base.h"
#include "actor_queue.h"
#include "actor_system_context.h"
#include <sys/epoll.h>


class Actor : public ActorBase {
public:

    Actor(std::shared_ptr<ActorSystemContext> context) : _ctx(context) {
        _poll_fd = epoll_create1(0);
    }

    virtual ~Actor() {
        this->stop();
    }

    void run() {
        _thread = std::move(std::thread([this]() { this->threadFn(); }));
    }

    void stop(bool flush = true) {
        _isShutdown.store(true);
        _flush.store(flush);
        if(_thread.joinable()){
            _thread.join();
        }
    }

    int getPollable() {
        return _poll_fd;
    }

    int addPollable(int pollable){
        epoll_event event;
        event.events = EPOLLIN | EPOLLET;
        _events.emplace_back();
        return epoll_ctl(_poll_fd, EPOLL_CTL_ADD, pollable, &event);
    }

    int removePollable(int pollable){
        epoll_event event;
        event.events = EPOLLIN | EPOLLET;
        return epoll_ctl(_poll_fd, EPOLL_CTL_DEL, pollable, &event);
    }

    UUID getUUID() {
        return _uuid;
    }

protected:
    UUID _uuid;
    std::shared_ptr<ActorSystemContext> _ctx;

private:
    std::atomic_bool _flush{true};
    std::atomic_bool _isShutdown{false};
    std::thread _thread;

    int _poll_fd;
    std::vector<epoll_event> _events;

    virtual void process() = 0;

    void threadFn() {
        while (!_isShutdown.load()) {
            epoll_wait(_poll_fd, &_events[0], _events.size(), -1); //wait for activity
            process();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        if(_flush.load()) {
            process();
        }
    }

};


#endif //LOOM_ACTOR_H
