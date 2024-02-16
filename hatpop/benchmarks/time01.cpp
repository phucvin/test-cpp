#include "../hatpop01.h"  // ?
// #include "../hatpop04.h"  // ?

#include "../third_party/ubench.h"

UBENCH(Time01, make_owned) {
    auto x = hatp::make_owned<int>(1);
}

UBENCH_MAIN();