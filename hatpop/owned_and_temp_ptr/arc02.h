#pragma once

#include <atomic>
#include <memory>
#include <cassert>

namespace {

thread_local std::map<void*, int> _trc_by_strong_arc;

// An attempt to achive thread-local/faster ref counting while being thread-safe
// References:
// - https://github.com/EricLBuehler/trc
// - https://stackoverflow.com/questions/70601992/c-thread-local-counter-implement
// - https://github.com/fereidani/rclite
// - https://github.com/facebook/folly/blob/main/folly/docs/ThreadLocal.md
// - https://stackoverflow.com/questions/36301420/is-it-possible-to-implement-boostthread-specific-ptr-via-thread-local
// - https://github.com/ANSANJAY/KernelPerCPUVariable
class WeakTrc {
public:
    std::atomic_int* arc_;
    WeakTrc(std::atomic_int* arc) : arc_(arc) {
        if (arc_ == nullptr) return;
        arc_[1].fetch_add(1);
    }
    ~WeakTrc() {
        if (arc_ == nullptr) return;
        if (arc_[1].fetch_sub(1) == 1 && arc_[0].load() == 0) {
            delete[] arc_;
        }
        arc_ = nullptr;
    }
};
class Trc {
public:
    std::atomic_int* arc_;

    Trc() : arc_(new std::atomic_int[2]{1, 0}) {}
    Trc(const WeakTrc& from) {
        arc_ = from.arc_;
        if (arc_ == nullptr) return;
        if (arc_[0].fetch_add(1) == 0) {
            arc_[0].fetch_sub(1);
            arc_ = nullptr;
        }
    }

    ~Trc() {
        if (arc_ == nullptr) return;
        if (arc_[0].fetch_sub(1) == 1) {
            
            delete[] arc_;
        }
        arc_ = nullptr;
    }

    void Reset() {
        arc_ = nullptr;
    }

    bool IsZero() { return arc_ == nullptr; }

    WeakTrc ToWeak() const {
        return WeakTrc(arc_);
    }

    // Returns false if already 0
    bool Inc() {
        if (internal_->arc.load() == 0) return false;
        if (internal_->trc == 0) assert(internal_->arc.fetch_add(1) > 0);
        ++internal_->trc;
        return true;
    }

    // Returns true if 0 after decrement
    bool Dec() {
        assert(--internal_->trc >= 0);
        if (internal_->trc > 0) return false;
        return internal_->arc.fetch_sub(1) == 1;
    }
};

}

namespace hatp {

template<typename T>
class TempPtr {
private:
    T* ptr_;
    Trc trc_;

public:
    TempPtr(Handle handle, WeakTrc trc)
            : ptr_(nullptr), trc_(trc) {
        if (trc_.IsZero()) return;
        if (!trc_.Inc()) {
            trc_.Reset();
            return;
        }
        ptr_ = (T*)HandleStore::GetSingleton()->GetUnsafe(handle);
        if (ptr_ == nullptr) {
            trc_.Dec();
            trc_.Reset();
            return;
        }
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

        auto tmp = ptr_;
        ptr_ = nullptr;
        if (trc_.Dec()) delete tmp;
        trc_.Reset();
    }
};

template<typename T>
class Unowned {
private:
    Handle handle_;
    WeakTrc trc_;

public:
    Unowned(Handle handle, const Trc& trc)
            : handle_(handle), trc_(trc.ToWeak()) {}

    TempPtr<T> GetTempPtr() const {
        return TempPtr<T>(handle_, trc_);
    }
};

template<typename T>
class Owned {
private:
    T* ptr_;
    Handle handle_;
    Trc trc_;

public:
    Owned(T* ptr) : ptr_(ptr) {
        if (ptr_) handle_ = HandleStore::GetSingleton()->Create(ptr_);
        else trc_.Reset();
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
        return Unowned<T>(handle_, trc_);
    }

    TempPtr<T> GetTempPtr() const {
        return TempPtr<T>(handle_, trc_.ToWeak());
    }

    void Release() {
        if (ptr_ == nullptr) return;
        
        HandleStore::GetSingleton()->Erase(handle_);
        auto tmp = ptr_;
        ptr_ = nullptr;
        handle_ = {};
        if (trc_.Dec()) delete tmp;
        trc_.Reset();
    }
};

}  // namespace hatp
