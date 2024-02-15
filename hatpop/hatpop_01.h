#pragma once

// Simple pointer handle store (i.e. the handle type is a raw pointer)
#include "handle_store/raw_pointer.h"
#include "common.h"
// Raw delete (i.e. `delete ptr`) when `Owned` releases
#include "owned_and_temp_ptr/raw_delete.h"