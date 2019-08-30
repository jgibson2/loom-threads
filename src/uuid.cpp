//
// Created by john on 7/24/19.
//
#include "uuid.h"

UUID::UUID() {
    std::random_device rd;
    for(auto& b : data.bytes) {
        b = rd();
    }
}

UUID::UUID(const UUID& other) {
    for(size_t i = 0; i < UUID_DATA_SIZE; ++i) {
        data.bytes[i] = other.data.bytes[i];
    }
}

UUID::UUID(UUID&& other) {
    for(size_t i = 0; i < UUID_DATA_SIZE; ++i) {
        data.bytes[i] = other.data.bytes[i];
    }
}

size_t UUID::hash() {
    return data.hash;
}