#include <iostream>
#include <string>
#include <thread>

#include "../hatpop_05.h"

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
            assert(foo->GetValue() != -1);
        }
    });
    std::jthread t2([&owned_foo] {
        owned_foo.Release();
    });
}

int main() {
    for (int i = 0; i < 100; ++i) race();
    return 0;
}
