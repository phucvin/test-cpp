#pragma once

#include <memory>
#include <cassert>

namespace {

// An attempt to achive thread-local/faster ref counting while being thread-safe
// References:
// - https://github.com/EricLBuehler/trc
class Trc {

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
            : ptr_(nullptr), trc_(trc.Upgrade()) {
        if (trc_ == nullptr) return;
        if (int prev = trc_->fetch_add(1); prev <= 0) {
            assert(trc_->fetch_sub(1) > 0);
            trc_.reset();
            return;
        }
        ptr_ = (T*)HandleStore::GetSingleton()->GetUnsafe(handle);
        if (ptr_ == nullptr) {
            assert(trc_->fetch_sub(1) > 0);
            trc_.reset();
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

        if (trc_->fetch_sub(1) <= 1) {
            auto tmp = ptr_;
            ptr_ = nullptr;
            delete tmp;
            trc_.reset();
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
    Owned(T* ptr) : ptr_(ptr), trc_(1) {
        if (ptr_) handle_ = HandleStore::GetSingleton()->Create(ptr_);
        else trc_.reset();
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
        return TempPtr<T>(handle_, trc_);
    }

    void Release() {
        if (ptr_ == nullptr) return;
        
        HandleStore::GetSingleton()->Erase(handle_);
        auto tmp = ptr_;
        ptr_ = nullptr;
        handle_ = {};
        if (trc_->fetch_sub(1) <= 1) delete tmp;
        trc_.reset();
    }
};

}  // namespace hatp
