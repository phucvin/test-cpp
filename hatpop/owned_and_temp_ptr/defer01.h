// References:
// - https://bzim.gitlab.io/blog/posts/incinerator-the-aba-problem-and-concurrent-reclamation.html

#pragma once

#include <atomic>
#include <vector>
#include <functional>

namespace {

std::atomic_int _current_read_count = 0;

class DeferredList {
private:
    std::vector<std::function<void()>> vec_;

public:
    ~DeferredList() {
        if (vec_.empty()) return;

        while (_current_read_count.load() > 0) continue;
        DeleteAll();
    }

    void Push(std::function<void()> deleter) { vec_.push_back(deleter); }

    void DeleteAll() {
        for (auto& deleter: vec_) deleter();
        vec_.clear();
    }
};

thread_local DeferredList _deferred_list;

void defer_enter() {
    _current_read_count.fetch_add(1);
}

void defer_exit() {
    _current_read_count.fetch_sub(1);
}

void defer_delete(std::function<void()> deleter) {
    _deferred_list.Push(std::move(deleter));
    if (_current_read_count.load() == 0) _deferred_list.DeleteAll();
}

}  // namespace

namespace hatp {

template<typename T>
class TempPtr {
private:
    T* ptr_;

public:
    TempPtr(Handle handle) : ptr_(nullptr) {
        void* tmp1 = HandleStore::GetSingleton()->GetUnsafe(handle);
        if (!tmp1) return;
        defer_enter();
        void* tmp2 = HandleStore::GetSingleton()->GetUnsafe(handle);
        if (tmp2 != tmp1) {
            defer_exit();
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

        defer_exit();
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
