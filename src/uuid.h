//
// Created by john on 7/17/19.
//

#ifndef LOOM_UUID_H
#define LOOM_UUID_H

#include <random>

#define UUID_DATA_SIZE sizeof(size_t)

class UUID {
public:
    UUID();

    UUID(const UUID& other);

    UUID(UUID&& other);

    size_t hash();

private:
    union { ;
        char bytes[UUID_DATA_SIZE];
        size_t hash;
    } data;
};


#endif //LOOM_UUID_H
