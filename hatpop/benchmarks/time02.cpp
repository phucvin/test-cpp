#include <atomic>
#include <barrier>
#include <future>
#include <iostream>
#include <vector>
#include <cassert>

// #include "../hatpop01.h"  // Error
// #include "../hatpop04.h"
// #include "../hatpop05.h"
// #include "../hatpop07.h"
#include "../hatpop11.h"

#include "../third_party/ubench.h"

std::vector<std::future<void>> make_futures() {
    return {};
}

void launch_async(std::vector<std::future<void>> &futures, std::function<void()> task) {
    futures.push_back(std::async(std::launch::async, task));
}

void wait_all(std::vector<std::future<void>> &futures) {
    for (const auto& f : futures) f.wait();
}

UBENCH(Time02, WarmUp) {
    auto futures = make_futures();

    for (int i = 0; i < 100; ++i) {
        launch_async(futures, [] { return; });
    }

    wait_all(futures);
}

UBENCH(Time02, Read1Release1) {
    auto owned_x = hatp::make_owned<int>(1);
    auto unowned_x = owned_x.GetUnowned();
    auto futures = make_futures();
    static std::atomic_int _printed = 0;

    launch_async(futures, [&] {
        if (auto x = unowned_x.GetTempPtr()) {
            if (*x != 1 && _printed.fetch_add(1) == 0) {
                std::cerr << "ERROR: *x != 1" << std::endl;
            }
        }
    });
    launch_async(futures, [&] {
        owned_x.Release();
    });

    wait_all(futures);
}

UBENCH(Time02, Read100_NoHatpop) {
    int x = 1;
    int* px = &x;
    auto futures = make_futures();

    for (int i = 0; i < 100; ++i) {
        launch_async(futures, [&] {
            assert(px != nullptr);
            assert(*px == 1);
        });
    }

    wait_all(futures);
}

UBENCH(Time02, Read100) {
    auto owned_x = hatp::make_owned<int>(1);
    auto unowned_x = owned_x.GetUnowned();
    auto futures = make_futures();

    for (int i = 0; i < 100; ++i) {
        launch_async(futures, [&] {
            auto x = unowned_x.GetTempPtr();
            assert(x);
            assert(*x == 1);
        });
    }

    wait_all(futures);
}

UBENCH(Time02, Read100WithBarrier) {
    auto owned_x = hatp::make_owned<int>(1);
    auto unowned_x = owned_x.GetUnowned();
    auto futures = make_futures();
    std::barrier bar(100);

    for (int i = 0; i < 100; ++i) {
        launch_async(futures, [&] {
            bar.arrive_and_wait();
            auto x = unowned_x.GetTempPtr();
            assert(x);
            assert(*x == 1);
        });
    }

    wait_all(futures);
}

void Time02_ReadNRelease1(int n) {
    auto owned_x = hatp::make_owned<int>(1);
    auto unowned_x = owned_x.GetUnowned();
    auto futures = make_futures();
    std::barrier bar(n);
    std::atomic_int success_reads = 0;
    static std::atomic_int _printed = 0;

    for (int i = 0; i < n-1; ++i) {
        launch_async(futures, [&] {
            bar.arrive_and_wait();
            if (auto x = unowned_x.GetTempPtr()) {
                success_reads.fetch_add(*x == 1 ? 1 : 0);
                if (*x != 1 && _printed.fetch_add(1) == 0) {
                    std::cerr << "ERROR: *x != 1" << std::endl;
                }
            }
        });
    }
    launch_async(futures, [&] {
        bar.arrive_and_wait();
        owned_x.Release();
    });

    wait_all(futures);
    if (_printed.fetch_add(1) < 3) {
        std::cout << "success_reads=" << success_reads << std::endl;
    }
}

UBENCH(Time02, Read1000Release1) {
    Time02_ReadNRelease1(1000);
}

UBENCH(Time02, Read100Release1) {
    Time02_ReadNRelease1(100);
}

UBENCH_MAIN();