#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>
#include <cassert>

class spinlock_relax {
public:
    spinlock_relax() : 
        flag{ATOMIC_FLAG_INIT} { }
    void lock() {
        // memory_order_relaxd: Operations with relax ordering doesn't participate in
        // synchronizes-with relationships. There is no guarntee that 
        // lock will happens
        // before unlock?
        while (flag.test_and_set(std::memory_order_relaxed));
    }
    void unlock() {
        flag.clear(std::memory_order_relaxed);
    }
private:
    std::atomic_flag flag;
};


class spinlock_safe {
public:
    spinlock_safe() : 
        flag{ATOMIC_FLAG_INIT} { }
    void lock() {
        // memory_order_relaxd: Operations with relax ordering doesn't participate in
        // synchronizes-with relationships. There is no guarntee that 
        // lock will happens
        // before unlock?
        while (flag.test_and_set(std::memory_order_acquire));
    }
    void unlock() {
        flag.clear(std::memory_order_release);
    }
private:
    std::atomic_flag flag;
};

class Object {
public:
    void increment() {
        std::lock_guard<decltype(lck)> lock(lck);
        x += 10;
    }

    int get_value() {
        std::lock_guard<decltype(lck)> lock(lck);
        return x;
    }
private:
    int x {0};
    spinlock_safe lck;
};

void play_with_spinlock() {

    const size_t num_of_threads = 100;
    std::vector<std::thread> threads;
    Object object;

    for (size_t i = 0; i < num_of_threads; i++) {
        threads.emplace_back([&]() { 
            object.increment();
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    assert(object.get_value() == 10 * num_of_threads);
}

/*
 * std::memory_order_relaxd can guarantee only that atomic variable in the same 
 * thread can't be reordered. There is no synchronizes-with relationship, and there
 * is no guarantee that order will be preserved among threads.
 */

void relaxed_ordering() {

    std::atomic<bool> x,y;
    std::atomic<int> z;

    x = false;
    y = false;
    z = 0;

    std::thread write_x_then_y([&]() {
        x.store(true, std::memory_order_relaxed); // 4
        y.store(true, std::memory_order_relaxed); // 1 
    });

    std::thread read_y_then_x([&]() {
        while (!y.load(std::memory_order_relaxed)); // 2
        if (x.load(std::memory_order_relaxed)) // 3
            ++z;
    });

    write_x_then_y.join();
    read_y_then_x.join();

    assert(z.load() != 0);

}

int main() {

    // play_with_spinlock();
    relaxed_ordering();

    return 0;
}
