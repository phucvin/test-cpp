#pragma once

// Simple pointer handle store (i.e. the handle type is a raw pointer)
#include "handle_store/raw_pointer.h"
#include "common_no_unowned.h"
// Atomic Reference Counter 
#include "owned_and_temp_ptr/arc.h"
// NOTE: Others don't work with raw_pointer, i.e. will get use-after-free errors,
// since they need to call HandleStore::GetUnsafe before doing further bookeeping
// ARC works in *most cases* (not yet tested for all possible cases) because it
// first checks the atomic ref count > 0 before calling HandleStore:GetUnsafe