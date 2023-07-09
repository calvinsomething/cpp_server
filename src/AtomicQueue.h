#pragma once

#include <condition_variable>
#include <mutex>

template <typename T, uint hard_size_limit>
class AtomicQueue {
    // Both queue and i are critical data
    T* queue;
    unsigned i, j, len;
    bool wrap;

    std::mutex mtx;
    std::condition_variable pop_condition;
    std::condition_variable push_condition;

    bool stopped;
public:
    AtomicQueue(unsigned len): i(), j(), len(len), wrap(), stopped() {
        if (len > hard_size_limit) len = hard_size_limit;
        queue = static_cast<T*>(alloca(len * sizeof(T)));
    }
    
    void push(T val) {
        std::unique_lock lock(mtx);
        while (wrap && i == j) {
            push_condition.wait(lock);
            if (stopped) return;
        }
        queue[i] = val;
        i = (i + 1) % len;
        wrap = !i;
        pop_condition.notify_one();
        return;
    }

    bool pop(T* output) {
        std::unique_lock lock(mtx);
        while (!wrap && j == i) {
            pop_condition.wait(lock);
            if (stopped) return false;
        }
        *output = queue[j];
        j = (j + 1) % len;
        wrap = wrap && j;
        push_condition.notify_one();
        return true;
    }

    void stop() {
        stopped = true;
        pop_condition.notify_all();
        push_condition.notify_all();
    }
};