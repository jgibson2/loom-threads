//
// Created by john on 7/17/19.
//

#ifndef LOOM_ACTOR_SYSTEM_CONTEXT_H
#define LOOM_ACTOR_SYSTEM_CONTEXT_H

#include <tuple>
#include <unistd.h>
#include "actor_queue.h"
#include "actor_base.h"
#include <boost/fiber/algo/algorithm.hpp>
#include <boost/fiber/fiber.hpp>
#include <boost/fiber/mutex.hpp>
#include <boost/fiber/algo/shared_work.hpp>
#include <boost/fiber/operations.hpp>
#include <boost/fiber/condition_variable.hpp>


class ActorSystemContext : public std::enable_shared_from_this<ActorSystemContext> {
public:
    ActorSystemContext() = default;

    template<typename ActorTypeT, typename... Args>
    std::shared_ptr<ActorTypeT> makeActor(bool flush, const Args&... args) {
        std::unique_lock<std::mutex> lock(_mut);
        auto ptr = std::make_shared<ActorTypeT>(shared_from_this(), args...);
        _actors.emplace_back(std::make_pair(flush, std::shared_ptr<ActorBase>(ptr)));
        ptr->run();
        return ptr;
    }

    template<typename ActorTypeT>
    std::shared_ptr<ActorTypeT> makeActor(bool flush = true) {
        std::unique_lock<std::mutex> lock(_mut);
        auto ptr = std::make_shared<ActorTypeT>(shared_from_this());
        _actors.emplace_back(std::make_pair(flush, std::shared_ptr<ActorBase>(ptr)));
        ptr->run();
        return ptr;
    }

    template<typename MessageTypeT, MessageTypeT DEFAULTVALUE = MessageTypeT()>
    std::shared_ptr<ActorQueue<MessageTypeT>> getQueue(const std::string& identifier) {
        if(_queueMap.find(identifier) == _queueMap.end()){
            makeQueue<MessageTypeT, DEFAULTVALUE>(identifier);
        }
        auto queue = _queueMap.at(identifier);
        auto* ptr = static_cast<ActorQueue<MessageTypeT, DEFAULTVALUE>*>(std::get<0>(queue));
        return std::shared_ptr<ActorQueue<MessageTypeT, DEFAULTVALUE>>(ptr,
                [](ActorQueue<MessageTypeT, DEFAULTVALUE>*){
                    //pass
                }
        );
    }

    void stop() {
        std::unique_lock<std::mutex> lock(_mut);

        for(auto& queue : _queueMap) {
            close(std::get<1>(queue.second)); //close fd associated with queue
        }

        for(auto& actor: _actors) {
            std::get<1>(actor)->stop(std::get<0>(actor));
        }
    }

    virtual ~ActorSystemContext() {
        this->stop();
    }

private:
    std::vector<std::tuple<bool, std::shared_ptr<ActorBase>>> _actors;
    //TODO: this is a memory leak
    //since the pointer is void*, destructors for the queue are not called
    std::unordered_map<std::string, std::tuple<void*, int>> _queueMap;
    std::mutex _mut;

    template<typename MessageTypeT, MessageTypeT DEFAULTVALUE = MessageTypeT()>
    std::shared_ptr<ActorQueue<MessageTypeT>> makeQueue(const std::string& identifier) {
        auto* ptr = new ActorQueue<MessageTypeT, DEFAULTVALUE>();
        _queueMap[identifier] = std::tuple<void*, int>(ptr, ptr->getPollable());
    }
};

#endif //LOOM_ACTOR_SYSTEM_CONTEXT_H
