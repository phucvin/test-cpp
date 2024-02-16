#pragma once

// Handle is the key returned by the slot map (high-performance container with
// unique keys) when inserting a pointer
#include "handle_store/slot_map.h"
#include "common.h"
// Defer deletion to the next retire when no threads are reading
#include "owned_and_temp_ptr/urcu02.h"