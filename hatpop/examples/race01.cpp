#include <barrier>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <cassert>

// #include "../hatpop01.h"  // Will fail since there is no sync before deleting
#include "../hatpop07.h"  // Will success

class Foo {
private:
    int value_;
public:
    explicit Foo(int value) : value_(value) {}
    ~Foo() { value_ = -1; }
    int GetValue() const { return value_; }
};

void race() {
    htp::Owned<Foo> owned_foo = htp::make_owned<Foo>(101);
    htp::Unowned<Foo> unowned_foo = owned_foo.GetUnowned();
    std::jthread t1([unowned_foo] {
        if (auto foo = unowned_foo.GetTempPtr(); foo) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            assert(foo->GetValue() == 101);
        }
    });
    std::jthread t2([&owned_foo] {
        owned_foo.Release();
    });
}

int main() {
    for (int i = 0; i < 10; ++i) race();
    std::cout << "Success!" << std::endl;
    return 0;
}
