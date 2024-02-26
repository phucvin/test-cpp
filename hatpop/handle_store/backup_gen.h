#pragma once

#include <utility>
#include <cstdlib>

namespace {

template <typename T>
struct HandledObject {
    uint8_t gen;
    T obj;

    template<typename ...Args>
    HandledObject(Args&& ...args) : obj(std::forward(args)...) {}
}

}

namespace hatp {

typedef void* Handle;

class HandleStore {
public:
    static HandleStore* GetSingleton();

    Handle Create(void* ptr) {
        return ptr;
    }

    template<typename Args>
    Handle Make(Args&& ...args) {
        auto ho = new HandledObject<T>(std::forward(args)...);
        ho.gen += 1;
        return Handle(ho.gen, &ho.obj);
    }

    void* GetUnsafe(Handle handle) {
        return handle;
    }

    void Erase(Handle handle) {
        return;
    }
};

namespace {
    HandleStore global_handle_store;
}  // namespace

HandleStore* HandleStore::GetSingleton() {
    return &global_handle_store;
}

}  // namespace hatp
