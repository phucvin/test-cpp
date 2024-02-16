#pragma once

#include <thread>
#include <functional>
#include <cstdlib>

namespace {

/*
NOTE: We can follow some simple implementation [1], but RCU will still have
deadlock issue (i.e. cannot call Owned.Release while holding TempPtr)

[1] https://concurrencyfreaks.blogspot.com/2016/09/a-simple-userspace-rcu-in-java.html
*/

constexpr size_t MAX_THREAD_COUNT = 10;
size_t _thread_read_times[MAX_THREAD_COUNT];  // Init values are 0s
std::atomic_int _clock = 1;

size_t tid() {
    size_t i = std::hash<std::thread::id>{}(std::this_thread::get_id());
    return i % MAX_THREAD_COUNT;
}

void urcu_read_lock() {
    _thread_read_times[tid()] = _clock.load();
}

void urcu_read_unlock() {
    _thread_read_times[tid()] = 0;
}

void urcu_sync() {
    int now = _clock.fetch_add(1);
    bool ok = false;
    while (!ok) {
        ok = true;
        for (size_t i = 0; i < MAX_THREAD_COUNT; ++i) {
            size_t t = _thread_read_times[i];
            if (t > 0 && t < now) {
                ok = false;
                break;
            }
        }
    }
}

}  // namespace

namespace htp {

template<typename T>
class TempPtr {
private:
    T* ptr_;

public:
    TempPtr(Handle handle) : ptr_(nullptr) {
        void* tmp1 = HandleStore::GetSingleton()->GetUnsafe(handle);
        if (!tmp1) return;
        urcu_read_lock();
        void* tmp2 = HandleStore::GetSingleton()->GetUnsafe(handle);
        if (tmp2 != tmp1) {
            urcu_read_unlock();
            return;
        }
        ptr_ = (T*)tmp2;
    }

    // This type is neither moveable nor copyable
    TempPtr(TempPtr&& rhs) = delete;
    TempPtr(const TempPtr&) = delete;
    ~TempPtr() { Release(); }
    T* operator *() const { return Get(); }
    T* operator ->() const { return Get(); }
    operator bool() const { return Get() != nullptr; }

    T* Get() const { return ptr_; }

    void Release() {
        if (ptr_ == nullptr) return;

        urcu_read_unlock();
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
        urcu_sync();
        delete ptr_;
        ptr_ = nullptr;
        handle_ = {};
    }
};

}  // namespace htp
