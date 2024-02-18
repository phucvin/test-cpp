#include <cassert>

#include "../hatpop11.h"
#include "../cached_unowned.h"

#include "../third_party/ubench.h"

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

UBENCH_EX(Time01, GetAndRelease100CachedTempPtr1M) {
    auto x = hatp::make_owned<int>(1);
    hatp::CachedUnowned<int> cached_unowned_x(x, 100);

    UBENCH_DO_BENCHMARK() {
        for (int i = 0; i < 1'000'000; ++i) {
            auto tp = cached_unowned_x.GetTempPtr();
            assert(tp);
            assert(*tp == 1);
        }
    }
}

UBENCH_MAIN();