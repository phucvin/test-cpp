// TODOs:
// - Copy / move temp ptr

#include <future>

#include "../hatpop07.h"

// Might randomly lead to use-after-free
void use_temp_ptr_in_another_thread() {
    auto x = hatp::make_owned<int>(1);
    if (auto tp = x.GetTempPtr()) {
        auto unused = std::async(std::launch::async, [&] {
            assert(*tp == 1);
        });
    }
    // This is similar to a bad practice of keeping reference/pointer to a local
    {
        int y = 2;
        auto unused = std::async(std::launch::async, [&] {
            assert(y == 2);
        });
    }
}

int main() {
    use_temp_ptr_in_another_thread();
    return 0;
}