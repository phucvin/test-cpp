#pragma once

// Handle is the key returned by the slot map (high-performance container with
// unique keys) when inserting a pointer
#include "slot_map_handle_store.h"
#include "common.h"
// Sub-optimal solution mimicking User-space RCU (read-copy-update) using a
// global shared_mutex (i.e. reader/writer lock) to wait/sync before deleting
// owned pointer
#include "global_shared_mutex_owned.h"