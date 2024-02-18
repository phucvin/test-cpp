#pragma once

#include <atomic>
#include <memory>
#include <cassert>

namespace hatp {

template<typename T>
class TempPtr {
private:
    T* ptr_;
    int* rc_;

public:
    TempPtr(Handle handle, int* rc) : ptr_(nullptr), rc_(rc) {
        if (*rc_ == 0) {
            rc_ = nullptr;
            return;
        }
        ptr_ = (T*)HandleStore::GetSingleton()->GetUnsafe(handle);
        if (ptr_ == nullptr) {
            rc_ = nullptr;
            return;
        }
        *rc_ += 1;
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
        delete tmp;
        if (--*rc_ == 0) delete rc_;
        rc_ = nullptr;
    }
};

template<typename T>
class Unowned {
private:
    Handle handle_;
    int* rc_;

public:
    Unowned(Handle handle, int* rc) : handle_(handle), rc_(rc) {}

    TempPtr<T> GetTempPtr() const {
        return TempPtr<T>(handle_, rc_);
    }
};

template<typename T>
class Owned {
private:
    T* ptr_;
    Handle handle_;
    int* rc_;

public:
    Owned(T* ptr) {
        if (ptr) {
            ptr_ = ptr;
            rc_ = new int(1);
            handle_ = HandleStore::GetSingleton()->Create(ptr_);
        }
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
        return Unowned<T>(handle_, rc_);
    }

    TempPtr<T> GetTempPtr() const {
        return TempPtr<T>(handle_, rc_);
    }

    void Release() {
        if (ptr_ == nullptr) return;
        
        HandleStore::GetSingleton()->Erase(handle_);
        auto tmp = ptr_;
        ptr_ = nullptr;
        handle_ = {};
        if (--*rc_ == 0) {
            delete rc_;
            delete tmp;
        }
        rc_ = nullptr;
    }
};

}  // namespace hatp
