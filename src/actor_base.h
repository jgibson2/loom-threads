//
// Created by john on 7/24/19.
//

#ifndef LOOM_ACTOR_BASE_H
#define LOOM_ACTOR_BASE_H


#include <thread>
#include <atomic>

class ActorBase {
public:
    virtual void stop(bool) = 0;
    virtual void run() = 0;
    virtual ~ActorBase() = default;
};


#endif //LOOM_ACTOR_BASE_H
