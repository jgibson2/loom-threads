cmake_minimum_required(VERSION 3.10)
project(loom CXX)

set(CMAKE_CXX_STANDARD 17)

add_library(loom STATIC src/actor.h src/actor_queue.h src/actor_system_context.h src/uuid.h src/uuid.cpp src/actor_base.h)

add_executable(loomtest test/test_runner.cpp test/actor_test.cpp test/actor_system_context_test.cpp test/queue_test.cpp)

target_link_libraries(loomtest pthread loom)

