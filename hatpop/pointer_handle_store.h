#pragma once

namespace htp {

typedef void* Handle;

class HandleStore {
public:
    static HandleStore* GetSingleton();

    Handle Create(void* ptr) {
        return ptr;
    }

    void* GetUnsafe(Handle handle) {
        return handle;
    }

    void Erase(Handle handle) {
        return;
    }
};

namespace {
    HandleStore global_pointer_handle_store;
}  // namespace

HandleStore* HandleStore::GetSingleton() {
    return &global_pointer_handle_store;
}

}  // namespace htp
