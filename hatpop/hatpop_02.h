#pragma once

// Simple array handle store (i.e. handle is the index of the inserted pointer
// in a mutex-protected vector)
#include "handle_store/array.h"
#include "common.h"
// Raw delete (i.e. `delete ptr`) when `Owned` releases
#include "owned_and_temp_ptr/raw_delete.h"