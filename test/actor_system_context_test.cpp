//
// Created by john on 8/19/19.
//

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