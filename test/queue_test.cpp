//
// Created by john on 8/28/19.
//

#include "../external/catch.hpp"
#include "../src/actor_queue.h"

TEST_CASE("QueueWorksTest", "[basic]") {
    ActorQueue<int> queue;
    REQUIRE(!queue.receive().has_value());

    REQUIRE(queue.send(1));
    REQUIRE(queue.receive().value() == 1);
}

TEST_CASE("BulkQueueWorksTest", "[basic]") {
    ActorQueue<int> queue;
    int total_sent = 10000;
    REQUIRE(!queue.receive().has_value());

    for(int i = 0; i < total_sent; ++i) {
        queue.send(i);
    }
    int total_received = 0;
    auto vec = queue.bulkReceive();
    while(!vec.empty()) {
        total_received += vec.size();
        vec = queue.bulkReceive();
    }
    REQUIRE(total_received == total_sent);
}

TEST_CASE("BulkQueueWorksInParallelTest", "[basic]") {
    ActorQueue<int> queue;
    int total_sent = 1000000;
    REQUIRE(!queue.receive().has_value());

    for(int i = 0; i < total_sent; ++i) {
        queue.send(i);
    }

    std::atomic_int total_received = 0;
    auto t1 = std::thread([&](){
        auto vec = queue.bulkReceive();
        while(!vec.empty()) {
            total_received += vec.size();
            vec = queue.bulkReceive();
        }
    });
    auto t2 = std::thread([&](){
        auto vec = queue.bulkReceive();
        while(!vec.empty()) {
            total_received += vec.size();
            vec = queue.bulkReceive();
        }
    });
    auto t3 = std::thread([&](){
        auto vec = queue.bulkReceive();
        while(!vec.empty()) {
            total_received += vec.size();
            vec = queue.bulkReceive();
        }
    });
    auto t4 = std::thread([&](){
        auto vec = queue.bulkReceive();
        while(!vec.empty()) {
            total_received += vec.size();
            vec = queue.bulkReceive();
        }
    });

    t1.join(); t2.join(); t3.join(); t4.join();

    REQUIRE(total_received == total_sent);
}