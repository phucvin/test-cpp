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
// #include "../hatpop06.h"  // OK
// #include "../hatpop07.h"  // OK
// #include "../hatpop08.h"  // OK
#include "../hatpop09.h"  // OK

int main() {
    hatp::Owned<int> owned_foo1 = hatp::make_owned<int>(1);
    hatp::Owned<int> owned_foo2 = hatp::make_owned<int>(2);
    std::jthread t1([&owned_foo1] {
        if (auto foo1 = owned_foo1.GetTempPtr()) {
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
