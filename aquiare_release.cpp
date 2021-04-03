#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <cassert>

void aquire_release_assert_can_fire() {

    std::atomic<bool> x { false };
    std::atomic<bool> y { false };
    std::atomic<int> z { 0 };

    // Nothing can move after release operation
    //
    // Nothing can move before acquire operation

    std::thread write_x([&]() {
        x.store(true, std::memory_order_release);
    });

    std::thread write_y([&]() {
        y.store(true, std::memory_order_release);
    });

    std::thread read_x_then_y([&]() {
        // The assertion below still can be fired. The reason is that the order of execution 
        // doesn't preserved in this example.
        // if thread (read_x_then_y) will execute first, the x.load should synchronize with the 
        // x.store, that is, the acquire HAVE to synchronize with the RELEASE on the same variable,
        // but the y.load() doesn't guarantee to synchronize with the write_y thread, thus it can
        // still read false ( see the initial value ). write_y may have not yet executed yet, and
        // it's in different thread
        while (!x.load(std::memory_order_acquire)); // while x is false, then spin
        if (y.load(std::memory_order_acquire)) // if y is true then increment the z
            ++z;
    });

    std::thread read_y_then_x([&]() {
        while (!y.load(std::memory_order_acquire)); // while y is false, then spin
        if (x.load(std::memory_order_acquire)) // if x is true then increment the z
            ++z;
    });

    write_x.join();
    write_y.join();
    read_x_then_y.join();
    read_y_then_x.join();

 
    // Assert here still can fire, because x, y store are happening from different
    // threads than the x, y loads
    assert(z != 0);
}

void aquire_release_assert_cannot_fire() {
    std::atomic<bool> x { false };
    std::atomic<bool> y { false };
    std::atomic<int> z { 0 };

    std::thread write_x_and_y([&]() {
        // relaxed here doesn't matter at all, because nothing can move after release
        // operation
        x.store(true, std::memory_order_relaxed);
        y.store(true, std::memory_order_release);
    });

    std::thread read_y_then_x([&]() {
        // Nothing can move before acquire operation
        while (!y.load(std::memory_order_acquire));
        if (x.load(std::memory_order_relaxed))
            ++z;
    });

    write_x_and_y.join();
    read_y_then_x.join();

    assert(z != 0);

}

int main() {

    aquire_release_assert_can_fire();
    // aquire_release_assert_cannot_fire();

    return 0;
}
