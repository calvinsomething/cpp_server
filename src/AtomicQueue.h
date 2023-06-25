#pragma once

#include <condition_variable>
#include <mutex>

template <typename T, uint hard_size_limit>
class AtomicQueue {
    // Both queue and i are critical data
    T* queue;
    size_t i, len; // TODO: ADD A SEPERATE INDEX FOR PUSHING AND POPPING SO IT'S A QUEUE, NOT A STACK!

    std::mutex mtx;
    std::condition_variable pop_condition;
    std::condition_variable push_condition;

    bool stopped;
public:
    AtomicQueue(unsigned len): i(), len(len), stopped(false) {
        if (len > hard_size_limit) len = hard_size_limit;
        queue = static_cast<T*>(alloca(len * sizeof(T)));
    }
    
    void push(T val) {
        std::unique_lock lock(mtx);
        while (i == len) {
            push_condition.wait(lock);
            if (stopped) return;
        }
        queue[i++] = val;
        pop_condition.notify_one();
        return;
    }

    bool pop(T* output) {
        std::unique_lock lock(mtx);
        while (!i) {
            pop_condition.wait(lock);
            if (stopped) return false;
        }
        *output = queue[--i];
        push_condition.notify_one();
        return true;
    }

    void stop() {
        stopped = true;
        pop_condition.notify_all();
        push_condition.notify_all();
    }
};