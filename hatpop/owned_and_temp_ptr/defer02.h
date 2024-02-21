// Similar to defer01, but using left right counters to avoid write-starvation when deleting

#pragma once

#include <atomic>
#include <vector>
#include <functional>

namespace {

std::atomic_int _read_count_index;
std::atomic_int _current_read_count[3];

class DeferredList {
private:
    std::vector<std::function<void()>> vec_;

public:
    ~DeferredList() {
        if (vec_.empty()) return;

        int prev_index = (_read_count_index.load() + 2) % 3;
        while (_current_read_count[prev_index].load() > 0) continue;
        int curr_index = _read_count_index.fetch_add(1) % 3;
        while (_current_read_count[curr_index].load() > 0) continue;
        DeleteAll();
    }

    void Push(std::function<void()> deleter) { vec_.push_back(deleter); }

    void DeleteAll() {
        for (auto& deleter: vec_) deleter();
        vec_.clear();
    }
};

thread_local DeferredList _deferred_list;

int defer_enter() {
    int index = _read_count_index.load() % 3;
    _current_read_count[index].fetch_add(1);
    return index;
}

void defer_exit(int index) {
    _current_read_count[index].fetch_sub(1);
}

void defer_delete(std::function<void()> deleter) {
    _deferred_list.Push(std::move(deleter));
    int curr_index = _read_count_index.fetch_add(1) % 3;
    if (_current_read_count[curr_index].load() == 0) _deferred_list.DeleteAll();
    int next_index = (curr_index + 1) % 3;
    assert(_current_read_count[next_index].load() == 0);
}

}  // namespace

namespace hatp {

template<typename T>
class TempPtr {
private:
    T* ptr_;
    int defer_index_;

public:
    TempPtr(Handle handle) : ptr_(nullptr) {
        void* tmp1 = HandleStore::GetSingleton()->GetUnsafe(handle);
        if (!tmp1) return;
        defer_index_ = defer_enter();
        void* tmp2 = HandleStore::GetSingleton()->GetUnsafe(handle);
        if (tmp2 != tmp1) {
            defer_exit(defer_index_);
            return;
        }
        ptr_ = (T*)tmp2;
    }

    // This type is neither moveable nor copyable
    TempPtr(TempPtr&& rhs) = delete;
    TempPtr(const TempPtr&) = delete;
    ~TempPtr() { Release(); }
    T operator *() const { return *Get(); }
    T* operator ->() const { return Get(); }
    operator bool() const { return Get() != nullptr; }

    T* Get() const { return ptr_; }

    void Release() {
        if (ptr_ == nullptr) return;

        defer_exit(defer_index_);
        ptr_ = nullptr;
    }
};

template<typename T>
class Owned {
private:
    T* ptr_;
    Handle handle_;

public:
    Owned(T* ptr) : ptr_(ptr) {
        if (ptr_) handle_ = HandleStore::GetSingleton()->Create(ptr_);
    }

    // This type is moveable but not copyable
    Owned(Owned&& rhs) {
        this->ptr_ = rhs.ptr_;
        this->handle_ = rhs.handle_;
        rhs.ptr_ = nullptr;
        rhs.handle_ = {};
    }
    // Common methods & operators
    Owned(const Owned&) = delete;
    ~Owned() { Release(); }
    operator Unowned<T>() { return GetUnowned(); }

    Unowned<T> GetUnowned() const {
        return Unowned<T>(handle_);
    }

    TempPtr<T> GetTempPtr() const {
        return TempPtr<T>(handle_);
    }

    void Release() {
        if (ptr_ == nullptr) return;
        
        HandleStore::GetSingleton()->Erase(handle_);
        auto tmp = ptr_;
        ptr_ = nullptr;
        handle_ = {};
        defer_delete([tmp]() { delete tmp; });
    }
};

}  // namespace hatp

