#include <atomic>
#include <iostream>
#include <cassert>

// #include "../hatpop01.h"  // Error
// #include "../hatpop04.h"
// #include "../hatpop05.h"
#include "../hatpop07.h"

#include "../third_party/thread_pool.h"
#include "../third_party/ubench.h"

constexpr int _max_threads = 101;
ctpl::thread_pool _thread_pool(_max_threads + 2);

UBENCH(Time02, WarmUp) {
    std::atomic_int done_count;
    for (int i = 0; i < _max_threads; ++i) {
        _thread_pool.push([&](int) {
            done_count.fetch_add(1);
        });
    }
    while (done_count.load() < _max_threads) continue;
}

UBENCH(Time02, Read1Release1) {
    auto owned_x = hatp::make_owned<int>(1);
    auto unowned_x = owned_x.GetUnowned();
    std::atomic_int done_count;
    static std::atomic_int _printed_err = 0;

    _thread_pool.push([&](int) {
        if (auto x = unowned_x.GetTempPtr()) {
            if (*x != 1 && _printed_err.fetch_add(1) == 0) {
                std::cerr << "ERROR: *x != 1" << std::endl;
            }
        }
        done_count.fetch_add(1);
    });
    _thread_pool.push([&](int) {
        owned_x.Release();
        done_count.fetch_add(1);
    });

    while (done_count.load() < 2) continue;
}

UBENCH(Time02, Read100) {
    auto owned_x = hatp::make_owned<int>(1);
    auto unowned_x = owned_x.GetUnowned();
    std::atomic_int done_count;

    for (int i = 0; i < _max_threads; ++i) {
        _thread_pool.push([&](int) {
            auto x = unowned_x.GetTempPtr();
            assert(x);
            assert(*x == 1);
            done_count.fetch_add(1);
        });
    }

    while (done_count.load() < _max_threads) continue;
}

UBENCH(Time02, Read100Release1) {
    auto owned_x = hatp::make_owned<int>(1);
    auto unowned_x = owned_x.GetUnowned();
    std::atomic_int done_count;
    static std::atomic_int _printed_err = 0;

    for (int i = 0; i < _max_threads; ++i) {
        if (i == _max_threads / 2) {
            _thread_pool.push([&](int) {
                owned_x.Release();
                done_count.fetch_add(1);
            });
        } else {
            _thread_pool.push([&](int) {
                if (auto x = unowned_x.GetTempPtr()) {
                    if (*x != 1 && _printed_err.fetch_add(1) == 0) {
                        std::cerr << "ERROR: *x != 1" << std::endl;
                    }
                }
                done_count.fetch_add(1);
            });
        }
    }

    while (done_count.load() < _max_threads) continue;
}

UBENCH_MAIN();