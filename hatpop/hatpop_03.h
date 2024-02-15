#pragma once

// Handle is the key returned by the slot map (high-performance container with
// unique keys) when inserting a pointer
#include "slot_map_handle_store.h"
#include "common.h"
// Raw delete (i.e. `delete ptr`) when `Owned` releases
#include "raw_delete_owned.h"