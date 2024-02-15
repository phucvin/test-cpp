#pragma once

// Handle is the key returned by the slot map (high-performance container with
// unique keys) when inserting a pointer
#include "slot_map_handle_store.h"
#include "common.h"
// User-space RCU (read-copy-update) to wait/sync for all readers to finish
// before deleting owned pointer
#include "urcu_owned.h"