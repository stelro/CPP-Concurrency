#include <atomic>
#include <cassert>
#include <iostream>
#include <mutex>
#include <thread>

int main() {

    // If you do a store with std::memory_order_release, and load from another thread
    // that reads the value with std::memory_order_acquire, then subsequent read operations
    // from the second thread will see any values stored to any memory location by the first
    // thread that were prior to the store-release, or a later store store to any of those memory
    // locations
    //
    // If both store and subsequent load are std::memory_order_seq_cst then the relationship
    // between those two threads is the same, you need more than two threads to see the 
    // difference.

    std::atomic<bool> x{false};
    std::atomic<bool> y{false};
    std::atomic<int> z{0};

    // So, in the example below, there is no relationship between store to x and store to y
    // so that it is possible for both the load of x and load of y to read false.
    // x and y are written by a different threads, so the ordering from the release to the acquire 
    // in each case has no effect on the operations in the other threads.
    //
    // If all the ordering is changed to std::memory_order_seq_cst, then this enforces an 
    // ordering between the stores to x and y. 
    //
    // The first guarantee of sequential consistency is that all operations will be executed
    // in the source code order. That's it, no store operation can overtake load operation

    std::thread write_x([&]() { 
        x.store(true, std::memory_order_release); 
    });

    std::thread write_y([&]() { 
        y.store(true, std::memory_order_release); 
    });

    std::thread read_x_then_y([&]() {
        while (!x.load(std::memory_order_acquire)); 
        if (y.load(std::memory_order_release))  // <-- If this reads true or false, 
        // that means that x store happened before
            ++z;
    });

    std::thread read_y_then_x([&]() {
        while (!y.load(std::memory_order_acquire));
        if (x.load(std::memory_order_release)) 
            ++z;
    });

    write_x.join();
    write_y.join();
    read_x_then_y.join();
    read_y_then_x.join();

    const auto z_value = z.load(std::memory_order_seq_cst);
    assert(z_value != 0);
    std::cout << "Z: " << z_value << std::endl;

    /*
     * z == 2? => 
     * write_x, write_y, read_y_then_x, read_x_then_y
     * read_x_then_y, write_x, write_y, read_y_then_x
     *
     * z == 1? =>
     * read_y_then_x, write_y, read_x_then_y, write_x
     * write_x, read_x_then_y, read_y_then_x, write_y
     * read_x_then_y, write_x, read_y_then_x, write_y
     *
     * z == 0? =>
     *
     */

    // Aquire-release memory ordering:
    // Ther is no total ordering between different threads
    // but there is synchronize with relationship between store and load of the same atomic variable
    
    

    return 0;
}
