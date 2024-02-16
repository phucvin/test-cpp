// #include "../hatpop01.h"
// #include "../hatpop04.h"
#include "../hatpop07.h"

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

UBENCH_EX(Time01, GetAndReleaseTempPtr) {
    auto x = hatp::make_owned<int>(1);

    UBENCH_DO_BENCHMARK() {
        auto temp_ptr = x.GetTempPtr();
        // temp_ptr.Release() is called when it goes out of scope
    }
}

UBENCH_EX(Time01, ReleaseOwned) {
    auto x = hatp::make_owned<int>(1);

    UBENCH_DO_BENCHMARK() {
        x.Release();
    }
}

UBENCH_MAIN();