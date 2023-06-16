#pragma once

#include <mutex>

template <typename T, uint hard_size_limit>
class AtomicQueue {
    // Both queue and i are critical data
    T* queue;
    size_t i, len;

    std::mutex mtx;
    std::condition_variable pop_condition;
    std::condition_variable push_condition;
public:
    AtomicQueue(unsigned len): i(), len(len) {
        if (len > hard_size_limit) len = hard_size_limit;
        queue = static_cast<T*>(alloca(len * sizeof(T)));
    }
    
    void push(T val) {
        std::unique_lock lock(mtx);
        if (i == len) {
            push_condition.wait(lock);
        }

        queue[i++] = val;
        pop_condition.notify_one();
        return;
    }

    T pop() {
        std::unique_lock lock(mtx);
        while (!i) {
            pop_condition.wait(lock);
        }

        T val = queue[--i];
        push_condition.notify_one();
        return val;
    }
};