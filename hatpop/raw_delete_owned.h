#pragma once

namespace htp {

template<typename T>
class TempPtr {
private:
    Handle handle_;
    T* ptr_;

public:
    TempPtr(Handle handle) : handle_(handle), ptr_(nullptr) {}
    // This type is neither moveable nor copyable
    TempPtr(TempPtr&& rhs) = delete;
    TempPtr(const TempPtr&) = delete;

    ~TempPtr() {
        Release();
    }

    T* operator *() const {
        return Get();
    }

    T* operator ->() const {
        return Get();
    }

    T* Get() const {
        return ptr_;
    }

    void Release() {
        ptr_ = nullptr;
    }
};

template<typename T>
class Owned {
private:
    T* ptr_;
    Handle handle_;

public:
    Owned(T* ptr, Handle handle) :
            ptr_(ptr),
            handle_(handle)
    {}

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

    Unowned<T> GetUnowned() const {
        return Unowned<T>(handle_);
    }

    TempPtr<T> GetTempPtr() const {
        return TempPtr<T>(handle_);
    }

    void Release() {
        if (ptr_ == nullptr) return;
        
        delete ptr_;
        ptr_ = nullptr;
    }
};

}  // namespace htp
