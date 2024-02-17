#pragma once

#include <atomic>
#include <memory>
#include <cassert>

namespace {

// An attempt to achive thread-local/faster ref counting while being thread-safe
// References:
// - https://github.com/EricLBuehler/trc
// - https://stackoverflow.com/questions/70601992/c-thread-local-counter-implement
// - https://github.com/fereidani/rclite
struct TrcInternal {
    std::atomic_int arc = 1;
    /*thread_local*/ int trc;
};
class WeakTrc {
public:
    std::weak_ptr<TrcInternal> internal_;
    WeakTrc(const std::shared_ptr<TrcInternal>& from) : internal_(from) {}
};
class Trc {
public:
    std::shared_ptr<TrcInternal> internal_;
    Trc() : internal_(std::make_shared<TrcInternal>()) { assert(Inc()); }
    Trc(const WeakTrc& from) : internal_(from.internal_.lock()) {}
    WeakTrc ToWeak() const { return WeakTrc(internal_); }
    bool IsZero() { return internal_ && internal_->arc.load() == 0; }
    void Reset() { internal_.reset(); }

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

        if (trc_.Dec()) {
            auto tmp = ptr_;
            ptr_ = nullptr;
            delete tmp;
            trc_.Reset();
        }
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
