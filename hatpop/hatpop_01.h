#pragma once

// Simple pointer handle store (i.e. the handle type is a raw pointer)
#include "pointer_handle_store.h"
#include "common.h"
// Raw delete (i.e. `delete ptr`) when `Owned` releases
#include "raw_delete_owned.h"