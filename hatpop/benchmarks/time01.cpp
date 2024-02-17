#include <cassert>

// #include "../hatpop01.h"
// #include "../hatpop04.h"
// #include "../hatpop07.h"
// #include "../hatpop09.h"
// #include "../hatpop11.h"
#include "../hatpop12.h"

#include "../third_party/ubench.h"

UBENCH(Time01, make_owned) {
    auto x = hatp::make_owned<int>(1);
}

UBENCH_EX(Time01, GetUnowned) {
    auto x = hatp::make_owned<int>(1);

    UBENCH_DO_BENCHMARK() {
        auto unowned = x.GetUnowned();
    }
}

UBENCH_EX(Time01, GetAndReleaseTempPtr1M) {
    auto x = hatp::make_owned<int>(1);

    UBENCH_DO_BENCHMARK() {
        for (int i = 0; i < 1'000'000; ++i) {
            auto tp = x.GetTempPtr();
            assert(tp);
            assert(*tp == 1);
        }
        // temp_ptr.Release() is called when it goes out of scope
    }
}

UBENCH_EX(Time01, GetAndReleaseTempPtr1M_NoHatpop) {
    int x = 1;
    int* px = &x;

    UBENCH_DO_BENCHMARK() {
        for (int i = 0; i < 1'000'000; ++i) {
            assert(px);
            assert(*px == 1);
        }
    }
}

UBENCH_EX(Time01, GetTempPtr1Access1M) {
    auto x = hatp::make_owned<int>(1);
    auto tp = x.GetTempPtr();

    UBENCH_DO_BENCHMARK() {
        for (int i = 0; i < 1'000'000; ++i) {
            assert(tp);
            assert(*tp == 1);
        }
    }
}

UBENCH_EX(Time01, ReleaseOwned) {
    auto x = hatp::make_owned<int>(1);

    UBENCH_DO_BENCHMARK() {
        x.Release();
    }
}

UBENCH_MAIN();