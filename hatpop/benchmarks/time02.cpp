#include <atomic>

#include "../hatpop01.h"
// #include "../hatpop04.h"
// #include "../hatpop07.h"

#include "../third_party/thread_pool.h"
#include "../third_party/ubench.h"

ctpl::thread_pool _thread_pool(2);

UBENCH(Time02, ReadRelease2Threads) {
    auto owned_x = hatp::make_owned<int>(1);
    auto unowned_x = owned_x.GetUnowned();
    std::atomic_int done_count;

    _thread_pool.push([&](int) {
        if (auto x = unowned_x.GetTempPtr()) {
            assert(*x == 1);
        }
        done_count.fetch_add(1);
    });
    _thread_pool.push([&](int) {
        owned_x.Release();
        done_count.fetch_add(1);
    });

    while (done_count.load() < 2) continue;
}

UBENCH_MAIN();