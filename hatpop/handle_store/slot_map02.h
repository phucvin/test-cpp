// An attempt to implement a simpler thread-safe & lock-free slot map than slot_map01.h
// References:
// - https://concurrencyfreaks.blogspot.com/2013/12/left-right-classical-algorithm.html?m=1
// - https://concurrencyfreaks.blogspot.com/2019/11/is-left-right-generic-concurrency.html?m=1
// - https://github.com/mmcshane/leftright

#pragma once

#include <atomic>
#include <cstdlib>

#include "../third_party/lockfree_queue.h"

namespace hatp {

typedef uintptr_t Handle;

class HandleStore {
private:
    std::atomic_int vec_idx_;
    std::vector<Handle> vec_[2];
    lockfree::mpmc::Queue<int> free_slots_;

public:
    static HandleStore* GetSingleton();

    Handle Create(void* ptr) {
        if (ptr == nullptr) return nullptr;
        if (int i; free_slots_.Pop(i)) {
            Handle h = Wrap(ptr, vec_[vec_idx_.load()][i]);
            vec_[vec_idx_.fetch_add()][i] = h;
            vec_[vec_idx_.load()][i] = h;
            return h;
        } else {
            Handle h = Wrap(ptr, nullptr);
            vec_[vec_idx_.fetch_add()].push_back(h);
            vec_[vec_idx_.load()][i].push_back(h);
            return h;
        }
    }

    void* GetUnsafe(Handle handle) {
        if (handle == nullptr) return nullptr;
        return Unwrap(handle, vec_[vec_idx_.load()]).ptr;
    }

    void Erase(Handle handle) {
        int i = Unwrap(handle, vec_[vec_idx_.load()]).i;
        if (i >= 0) assert(free_slots.Push(i));
    }
};

namespace {
    HandleStore global_handle_store;
}  // namespace

HandleStore* HandleStore::GetSingleton() {
    return &global_handle_store;
}

}  // namespace hatp
