#include "../third_party/ubench.h"

UBENCH(foo, bar) {
  usleep(100 * 1000);
}

UBENCH_MAIN();