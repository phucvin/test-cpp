#pragma once

#include <mutex>

#include "third_party/slot_map.h"

namespace htp {

typedef dod::slot_map_key64<void*> Handle;

class HandleStore {
private:
    // TODO: If mutex is really needed, find & use a lock-free slot_map instead
    std::mutex mu_;
    dod::slot_map64<void*> slot_map_;

public:
    static HandleStore* GetSingleton();

    Handle Create(void* ptr) {
        std::lock_guard<std::mutex> l(mu_);
        return slot_map_.emplace(ptr);
    }

    void* GetUnsafe(Handle handle) {
        std::lock_guard<std::mutex> l(mu_);
        void** pptr = slot_map_.get(handle);
        if (pptr == nullptr) return nullptr;
        else return *pptr;
    }

    void Erase(Handle handle) {
        std::lock_guard<std::mutex> l(mu_);
        slot_map_.erase(handle);
    }
};

namespace {
    HandleStore global_handle_store;
}  // namespace

HandleStore* HandleStore::GetSingleton() {
    return &global_handle_store;
}

}  // namespace htp
