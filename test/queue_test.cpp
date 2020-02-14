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
