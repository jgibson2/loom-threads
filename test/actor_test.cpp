//
// Created by john on 7/17/19.
//

#include "../external/catch.hpp"
#include "../src/actor.h"

class TestActor : public Actor {
public:
    TestActor(std::shared_ptr<ActorSystemContext> ctx) : Actor::Actor(ctx) {
    }

private:

    void process() {
            std::cout << "Running in thread " << std::this_thread::get_id() << std::endl;
    }
};

class QueueTestActor : public Actor {
public:
    static const std::string QueueIDString;
    QueueTestActor(std::shared_ptr<ActorSystemContext> ctx, int i) : Actor::Actor(ctx), _i(i) {
        _intQueue =_ctx->getQueue<int>(QueueIDString);
        addPollable(_intQueue->getPollable());
    }

    virtual ~QueueTestActor() = default;

private:
    int _i;
    std::shared_ptr<ActorQueue<int>> _intQueue;

    void process() final {
        auto _intQueue = _ctx->getQueue<int>(QueueIDString);
        auto opt = _intQueue->receive();
        while(opt.has_value()) {
            std::cout << "Running in thread " << std::this_thread::get_id();
            std::cout << " and got from queue " << opt.value() << std::endl;
            opt = _intQueue->receive();
        }
    }
};

const std::string QueueTestActor::QueueIDString = "test";

TEST_CASE("ActorWorks", "[basic]") {
    auto ctx = std::make_shared<ActorSystemContext>();
    ctx->makeActor<QueueTestActor>(true, 10);
    auto queue = ctx->getQueue<int>(QueueTestActor::QueueIDString);
    for(int i = 0; i < 20; ++i) {
        queue->send(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}