// #include "../hatpop01.h"
// #include "../hatpop04.h"
#include "../hatpop07.h"

#include "../third_party/ubench.h"

UBENCH(Time02, ReadRelease2Threads) {
    auto x = hatp::make_owned<int>(1);
}

UBENCH_MAIN();