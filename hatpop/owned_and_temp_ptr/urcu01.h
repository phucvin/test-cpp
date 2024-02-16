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
// Read indicators for each thread slot, first element is the read version (0
// indicating no reading), second element is a counter indicating how many
// readers/threads are using the same slot for reading
// Init values are 0s
std::atomic_int _read_indicators[_thread_slots][2];
std::atomic_int _writer_version = 1;

int slot() {
    unsigned int u = std::hash<std::thread::id>{}(std::this_thread::get_id());
    return u % _thread_slots;
}

void urcu_read_lock() {
    int s = slot();
    std::atomic_int& read_version = _read_indicators[s][0];
    std::atomic_int& read_counter = _read_indicators[s][1];

    // Register new reader
    assert(read_counter.fetch_add(1) >= 0);
    int writer_version = _writer_version.load();
    int expected_read_version = 0;
    // Save writer version if the current read version is 0 (i.e. not reading)
    // Keep the current read version if non-0, this happens when the current slot
    // is being used for another read in the past
    if (!read_version.compare_exchange_strong(
            expected_read_version, writer_version)) {
        assert(expected_read_version <= writer_version);
    }
}

void urcu_read_unlock() {
    int s = slot();
    std::atomic_int& read_version = _read_indicators[s][0];
    std::atomic_int& read_counter = _read_indicators[s][1];

    // Unregister reader
    int current_read_counter = read_counter.fetch_sub(1);
    assert(current_read_counter >= 0);
    if (current_read_counter > 1) return;
    // If potentially last reader, set read version to 0 (not reading)
    int current_read_version = read_version.load();
    int expected_read_version = current_read_version;
    // It's OK to fail setting to 0, since another thread might start using this
    // slot for new read (so it must be true that new read version > current read version)
    if (!read_version.compare_exchange_strong(expected_read_version, 0)) {
        assert(expected_read_version > current_read_version);
    }
}

void urcu_sync() {
    int writer_version = _writer_version.fetch_add(1);
    for (int s = 0; s < _thread_slots; ++s) {
        std::atomic_int& read_version = _read_indicators[s][0];
        for (int v = 1; v > 0 && v <= writer_version; v = read_version.load())
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
