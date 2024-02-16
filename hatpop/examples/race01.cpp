#include <barrier>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <cassert>

// #include "../hatpop01.h"  // Fail since there is no sync before deleting
#include "../hatpop04.h"  // OK
// #include "../hatpop07.h"  // OK
// #include "../hatpop09.h"  // OK
// #include "../hatpop10.h"  // WIP

class Foo {
private:
    int value_;
public:
    explicit Foo(int value) : value_(value) {}
    ~Foo() { value_ = -1; }
    int GetValue() const { return value_; }
};

void race() {
    hatp::Owned<Foo> owned_foo = hatp::make_owned<Foo>(101);
    hatp::Unowned<Foo> unowned_foo = owned_foo.GetUnowned();
    std::jthread t1([unowned_foo] {
        if (auto foo = unowned_foo.GetTempPtr(); foo) {
            // std::this_thread::sleep_for(std::chrono::milliseconds(1));
            assert(foo->GetValue() == 101);
        }
    });
    std::jthread t2([&owned_foo] {
        owned_foo.Release();
    });
}

int main() {
    for (int i = 0; i < 100'000; ++i) race();
    std::cout << "Success!" << std::endl;
    return 0;
}
