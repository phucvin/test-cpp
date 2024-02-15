#pragma once

#include <cstdlib>
#include <mutex>
#include <vector>

namespace htp {

typedef size_t Handle;

class HandleStore {
private:
    std::mutex mu_;
    std::vector<void*> vec_;

public:
    static HandleStore* GetSingleton();

    Handle Create(void* ptr) {
        std::lock_guard<std::mutex> l(mu_);
        vec_.push_back(ptr);
        return vec_.size() - 1;
    }

    void* GetUnsafe(Handle handle) {
        std::lock_guard<std::mutex> l(mu_);
        auto i = (size_t)handle;
        if (i < 0 || i >= vec_.size()) return nullptr;
        else return vec_[i];
    }

    void Erase(Handle handle) {
        std::lock_guard<std::mutex> l(mu_);
        auto i = (size_t)handle;
        if (i < 0 || i >= vec_.size()) return;
        else vec_[i] = nullptr;
    }
};

namespace {
    HandleStore global_handle_store;
}  // namespace

HandleStore* HandleStore::GetSingleton() {
    return &global_handle_store;
}

}  // namespace htp
