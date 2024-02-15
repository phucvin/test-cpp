#pragma once

// Simple array handle store (i.e. handle is the index of the inserted pointer
// in a mutex-protected vector)
#include "array_handle_store.h"
#include "common.h"
// Raw delete (i.e. `delete ptr`) when `Owned` releases
#include "raw_delete_owned.h"