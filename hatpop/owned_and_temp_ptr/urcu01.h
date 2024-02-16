// References:
// - https://concurrencyfreaks.blogspot.com/2016/09/a-simple-userspace-rcu-in-java.html

#pragma once

#include <thread>
#include <functional>

namespace {

#ifndef HATPOP_URCU01_THREAD_SLOTS
#define HATPOP_URCU01_THREAD_SLOTS 4;
#endif
constexpr int _thread_slots = HATPOP_URCU01_THREAD_SLOTS;
std::atomic_int _read_indicators[_thread_slots][2];  // Init values are 0s
std::atomic_int _clock = 1;

int slot() {
    unsigned int u = std::hash<std::thread::id>{}(std::this_thread::get_id());
    return u % _thread_slots;
}

void urcu_read_lock() {
    int s = slot();
    std::atomic_int& s_clock = _read_indicators[s][0];
    std::atomic_int& s_counter = _read_indicators[s][1];

    int now = _clock.load();
    assert(s_counter.fetch_add(1) >= 0);
    int expected_s_clock = 0;
    if (!s_clock.compare_exchange_strong(expected_s_clock, now)) {
        assert(expected_s_clock < now);
    }
}

void urcu_read_unlock() {
    int s = slot();
    std::atomic_int& s_clock = _read_indicators[s][0];
    std::atomic_int& s_counter = _read_indicators[s][1];
    int current_s_counter = s_counter.fetch_sub(1);
    assert(current_s_counter >= 0);
    if (current_s_counter == 1) {
        int current_s_clock = s_clock.load();
        int expected_s_clock = current_s_clock;
        if (!s_clock.compare_exchange_strong(expected_s_clock, 0)) {
            assert(expected_s_clock > current_s_clock);
        }
    }
}

void urcu_sync() {
    int now = _clock.fetch_add(1);
    for (int s = 0; s < _thread_slots; ++s) {
        std::atomic_int& s_clock = _read_indicators[s][0];
        for (int t = 1; t > 0 && t <= now; t = s_clock.load())
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
