#pragma once

// Handle is the key returned by the slot map (high-performance container with
// unique keys) when inserting a pointer
#include "handle_store/slot_map.h"
#include "common.h"
// Raw delete (i.e. `delete ptr`) when `Owned` releases
#include "owned_and_temp_ptr/raw_delete.h"