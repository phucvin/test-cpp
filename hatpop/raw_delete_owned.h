#pragma once

namespace htp {

void* HandleStore_GetSafe(Handle handle) {
    return HandleStore::GetSingleton()->GetUnsafe(handle);
}

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
        
        HandleStore::GetSingleton()->Erase(handle_);
        delete ptr_;
        ptr_ = nullptr;
    }
};

}  // namespace htp
