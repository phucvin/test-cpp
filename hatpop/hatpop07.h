#pragma once

// Handle is the key returned by the slot map (high-performance container with
// unique keys) when inserting a pointer
#include "handle_store/slot_map01.h"
#include "common_no_unowned.h"
// Atomic Reference Counter 
#include "owned_and_temp_ptr/arc01.h"