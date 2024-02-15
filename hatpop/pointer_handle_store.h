#include "interfaces.h"

namespace htp {

namespace internal {

class PointerHandleStore : public IHandleStore {
public:
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

PointerHandleStore global_pointer_handle_store;

}  // namespace internal

IHandleStore* IHandleStore::GetSingleton() {
    return &internal::global_pointer_handle_store;
}

}  // namespace htp
