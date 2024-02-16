// References:
// - https://concurrencyfreaks.blogspot.com/2016/09/a-simple-userspace-rcu-in-java.html

#pragma once

#include <thread>
#include <functional>

namespace {

constexpr int MAX_THREAD_COUNT = 128;
std::atomic_int _thread_read_times[MAX_THREAD_COUNT];  // Init values are 0s
std::atomic_int _clock = 1;

int tid() {
    unsigned int u = std::hash<std::thread::id>{}(std::this_thread::get_id());
    return u % MAX_THREAD_COUNT;
}

void urcu_read_lock() {
    int t1 = _clock.load();
    _thread_read_times[tid()].store(t1);
    // The following is probably needed per reference
    /*
    int t2 = _clock.load();
    if (t2 != t1) _thread_read_times[tid()].store(t2);
    */
}

void urcu_read_unlock() {
    _thread_read_times[tid()].store(0);
}

void urcu_sync() {
    int now = _clock.fetch_add(1);
    for (int i = 0; i < MAX_THREAD_COUNT; ++i) {
        for (int t = 1; t > 0 && t <= now; t = _thread_read_times[i].load())
            continue;
    }
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
        auto tmp = ptr_;
        ptr_ = nullptr;
        handle_ = {};
        urcu_sync();
        delete tmp;
    }
};

}  // namespace hatp
