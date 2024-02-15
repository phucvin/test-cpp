#pragma once

// Handle is the key returned by the slot map (high-performance container with
// unique keys) when inserting a pointer
#include "handle_store/slot_map.h"
#include "common_no_unowned.h"
// Sub-optimal solution mimicking User-space RCU (read-copy-update) using
// shared_mutex (i.e. reader/writer lock) to wait/sync before deleting
// owned pointer
#include "owned_and_temp_ptr/shared_mutex.h"