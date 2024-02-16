#include <chrono>
#include <iostream>
#include <thread>

// Output would be:
// Releasing foo2...
// Done keeping foo1!
// Released foo2!
// #include "../hatpop05.h"  // Worse, releasing foo2 has to wait for foo1

// Output would be:
// Releasing foo2...
// Released foo2!
// Done keeping foo1!
#include "../hatpop06.h"  // Better

int main() {
    htp::Owned<int> owned_foo1 = htp::make_owned<int>(1);
    htp::Owned<int> owned_foo2 = htp::make_owned<int>(2);
    std::jthread t1([&owned_foo1] {
        if (auto foo1 = owned_foo1.GetTempPtr(); foo1) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout << "Done keeping foo1!" << std::endl;
        }
    });
    std::jthread t2([&owned_foo2] {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        std::cout << "Releasing foo2..." << std::endl;
        owned_foo2.Release();
        std::cout << "Released foo2!" << std::endl;
    });
    return 0;
}
