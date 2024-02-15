#pragma once

// Handle is the key returned by the slot map (high-performance container with
// unique keys) when inserting a pointer
#include "handle_store/slot_map.h"
#include "common.h"
// User-space RCU (read-copy-update) to wait/sync for all readers to finish
// before deleting owned pointer
#include "owned_and_temp_ptr/urcu01.h"