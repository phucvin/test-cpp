#pragma once

#include <shared_mutex>
#include <memory>

namespace hatp {

template<typename T>
class TempPtr {
private:
    T* ptr_;
    std::shared_ptr<std::shared_mutex> mu_;

public:
    TempPtr(Handle handle, std::shared_ptr<std::shared_mutex> mu)
            : ptr_(nullptr), mu_(std::move(mu)) {
        void* tmp1 = HandleStore::GetSingleton()->GetUnsafe(handle);
        if (!tmp1) {
            mu_.reset();
            return;
        }
        mu_->lock_shared();
        void* tmp2 = HandleStore::GetSingleton()->GetUnsafe(handle);
        if (tmp2 != tmp1) {
            mu_->unlock_shared();
            mu_.reset();
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
    std::shared_ptr<std::shared_mutex> mu_;

public:
    Unowned(Handle handle, std::shared_ptr<std::shared_mutex> mu)
            : handle_(handle), mu_(std::move(mu)) {}

    TempPtr<T> GetTempPtr() const {
        return TempPtr<T>(handle_, mu_);
    }
};

template<typename T>
class Owned {
private:
    T* ptr_;
    Handle handle_;
    std::shared_ptr<std::shared_mutex> mu_;

public:
    Owned(T* ptr) : ptr_(ptr), mu_(std::make_shared<std::shared_mutex>()) {
        if (ptr_) handle_ = HandleStore::GetSingleton()->Create(ptr_);
        else mu_.reset();
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
        return Unowned<T>(handle_, mu_);
    }

    TempPtr<T> GetTempPtr() const {
        return TempPtr<T>(handle_, mu_);
    }

    void Release() {
        if (ptr_ == nullptr) return;
        
        HandleStore::GetSingleton()->Erase(handle_);
        auto tmp = ptr_;
        ptr_ = nullptr;
        handle_ = {};
        mu_->lock();
        mu_->unlock();
        delete tmp;
    }
};

}  // namespace hatp
