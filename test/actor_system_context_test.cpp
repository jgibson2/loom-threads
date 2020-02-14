//
// Created by john on 8/19/19.
//

#include <sstream>
#include "../external/catch.hpp"
#include "../src/actor_system_context.h"

TEST_CASE("ActorSystemContextWorks", "[basic]") {
    auto ctx = std::make_shared<ActorSystemContext>();
    {
        auto queue = ctx->getQueue<int>("test");
        queue->send(1);
        queue->send(2);
        queue->send(3);
    }
    {
        auto queue = ctx->getQueue<int>("test");
        REQUIRE(queue->receive().value() == 1);
        REQUIRE(queue->receive().value() == 2);
        REQUIRE(queue->receive().value() == 3);
        REQUIRE(!queue->receive().has_value());
    }
}

TEST_CASE("ActorSystemContextWorksWithHeldReference", "[basic]") {
    auto ctx = std::make_shared<ActorSystemContext>();
    auto outerQueue = ctx->getQueue<int>("test");
    {
        auto queue = ctx->getQueue<int>("test");
        queue->send(1);
        queue->send(2);
        queue->send(3);
    }
    {
        REQUIRE(outerQueue->receive().value() == 1);
        REQUIRE(outerQueue->receive().value() == 2);
        REQUIRE(outerQueue->receive().value() == 3);
        REQUIRE(!outerQueue->receive().has_value());
    }
}

TEST_CASE("ActorSystemContextQueueFailsOnBadType", "[basic]") {
    auto ctx = std::make_shared<ActorSystemContext>();
    auto q1 = ctx->getQueue<int>("test");
    REQUIRE_THROWS(
             ctx->getQueue<long>("test")
            );
}


TEST_CASE("ActorSystemQueuesWorkWithMultipleThreads", "[basic]") {
    auto ctx = std::make_shared<ActorSystemContext>();
    std::vector<std::thread> threads;
    for(int i = 0; i < 16; ++i) {
        threads.emplace_back([&]{
//            auto id = std::this_thread::get_id();
            auto id = "Test";
            std::stringstream ss;
            ss << id;
            std::string str = ss.str();
            auto queue = ctx->getQueue<int>(str);
            for(int j = 0; j < 1000; ++j) {
                queue->send(j);
            }
            for(int j = 0; j < 1000; ++j) {
                auto t = queue->receive();
                if(!t.has_value()){
                    exit(8);
                }
            }
        });
    }

    for(auto& t : threads) {
        t.join();
    }
}

TEST_CASE("ActorSystemContextWorksWithMultipleThreads", "[basic]") {
    auto ctx = std::make_shared<ActorSystemContext>();
    std::vector<std::thread> threads;
    for(int i = 0; i < 16; ++i) {
        threads.emplace_back([&]{
            auto id = std::this_thread::get_id();
            std::stringstream ss;
            ss << id;
            std::string str = ss.str();
            auto queue = ctx->getQueue<int>(str);
            for(int j = 0; j < 1000; ++j) {
                queue->send(j);
            }
            for(int j = 0; j < 1000; ++j) {
                auto t = queue->receive();
                //can't usse REQUIRE here since Catch does NOT like multiple threads calling it at once
                if(!t.has_value()){
                    exit(8);
                }
            }
        });
    }

    for(auto& t : threads) {
        t.join();
    }
}