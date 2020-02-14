//
// Created by john on 7/17/19.
//

#ifndef LOOM_ACTOR_SYSTEM_CONTEXT_H
#define LOOM_ACTOR_SYSTEM_CONTEXT_H

#include <tuple>
#include <unistd.h>
#include <memory>
#include <typeindex>
#include <mutex>
#include <shared_mutex>
#include "actor_queue.h"
#include "actor_base.h"


class bad_type_request : public std::logic_error {
public:
    bad_type_request(const char* t1, const char* t2) : std::logic_error("Bad Type Requested") {
        _t1_str = t1;
        _t2_str = t2;
    }

    virtual const char* what() const noexcept {
        auto s = std::string("ERROR! Bad Type Requested: Requested Type: ")
                + _t1_str
                + std::string(" Actual type: ")
                + _t2_str;
        return s.c_str();
    }
private:
    const char* _t1_str;
    const char* _t2_str;
};


class ActorSystemContext : public std::enable_shared_from_this<ActorSystemContext> {
public:
    using QueueAndFDWithTypeInfo = std::tuple<std::shared_ptr<ActorQueueBase>, int, size_t>;

    ActorSystemContext() = default;

    template<typename ActorTypeT, typename... Args>
    std::shared_ptr<ActorTypeT> makeActor(bool flush, const Args&... args) {
        std::lock_guard<std::mutex> writeLock(_vectorMut);
        auto ptr = std::make_shared<ActorTypeT>(shared_from_this(), args...);
        _actors.emplace_back(std::make_pair(flush, std::shared_ptr<ActorBase>(ptr)));
        ptr->run();
        return ptr;
    }

    template<typename ActorTypeT>
    std::shared_ptr<ActorTypeT> makeActor(bool flush = true) {
        std::lock_guard<std::mutex> writeLock(_vectorMut);
        auto ptr = std::make_shared<ActorTypeT>(shared_from_this());
        _actors.emplace_back(std::make_pair(flush, std::shared_ptr<ActorBase>(ptr)));
        ptr->run();
        return ptr;
    }

    template<typename MessageTypeT, MessageTypeT DEFAULTVALUE = MessageTypeT()>
    std::shared_ptr<ActorQueue<MessageTypeT>> getQueue(const std::string& identifier) {
        std::shared_lock<std::shared_mutex> readLock(_mapMut);
        if(_queueMap.find(identifier) == _queueMap.end()){
            readLock.unlock();
            makeQueue<MessageTypeT, DEFAULTVALUE>(identifier);
            readLock.lock();
        }
        auto& queue = _queueMap.at(identifier);
        auto qPtr = std::get<0>(queue);
        if(std::type_index(typeid(MessageTypeT)).hash_code() != std::get<2>(queue)){
            throw bad_type_request(std::type_index(typeid(MessageTypeT)).name(), _typeMap.at(std::get<2>(queue)).c_str());
        }
        auto dPtr = std::dynamic_pointer_cast<ActorQueue<MessageTypeT, DEFAULTVALUE>>(qPtr);
        return dPtr;
    }

    void stop() {
        std::lock_guard<std::shared_mutex> writeLock(_mapMut);
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
    std::unordered_map<std::string, QueueAndFDWithTypeInfo> _queueMap;
    std::unordered_map<size_t, std::string> _typeMap;
    std::shared_mutex _mapMut;
    std::mutex _vectorMut;

    template<typename MessageTypeT, MessageTypeT DEFAULTVALUE = MessageTypeT()>
    std::shared_ptr<ActorQueue<MessageTypeT>> makeQueue(const std::string& identifier) {
        std::lock_guard<std::shared_mutex> writeLock(_mapMut);
        auto q = std::make_shared<ActorQueue<MessageTypeT, DEFAULTVALUE>>();
        _queueMap[identifier] = std::make_tuple(q, q->getPollable(), q->typeIndex.hash_code());
        _typeMap[q->typeIndex.hash_code()] = q->typeIndex.name();
        return q;
    }

};

#endif //LOOM_ACTOR_SYSTEM_CONTEXT_H
