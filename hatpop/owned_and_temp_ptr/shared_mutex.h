#pragma once

#include <shared_mutex>

namespace htp {

template<typename T>
class TempPtr {
private:
    T* ptr_;
    std::shared_mutex* mu_;

public:
    TempPtr(Handle handle, std::shared_mutex* mu) : ptr_(nullptr), mu_(mu) {
        void* tmp1 = HandleStore::GetSingleton()->GetUnsafe(handle);
        if (!tmp1) return;
        mu_->lock_shared();
        void* tmp2 = HandleStore::GetSingleton()->GetUnsafe(handle);
        if (tmp2 != tmp1) {
            mu_->unlock_shared();
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

        mu_->unlock_shared();
        ptr_ = nullptr;
    }
};

template<typename T>
class Unowned {
private:
    Handle handle_;
    std::shared_mutex* mu_;

public:
    Unowned(Handle handle, std::shared_mutex* mu) : handle_(handle), mu_(mu) {}

    TempPtr<T> GetTempPtr() const {
        return TempPtr<T>(handle_, mu_);
    }
};

template<typename T>
class Owned {
private:
    T* ptr_;
    Handle handle_;
    std::shared_mutex mu_;

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
    Owned(const Owned&) = delete;

    ~Owned() {
        Release();
    }

    Unowned<T> GetUnowned() {
        return Unowned<T>(handle_, &mu_);
    }

    TempPtr<T> GetTempPtr() {
        return TempPtr<T>(handle_, &mu_);
    }

    void Release() {
        if (ptr_ == nullptr) return;
        
        HandleStore::GetSingleton()->Erase(handle_);
        auto tmp = ptr_;
        ptr_ = nullptr;
        handle_ = {};
        mu_.lock();
        mu_.unlock();
        delete tmp;
    }
};

}  // namespace htp
