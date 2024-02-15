#include "interfaces.h"

namespace htp {

template<typename T>
class TempPtr : public ITempPtr<T> {
private:
    T* ptr_;

public:
    TempPtr(T* ptr) : ptr_(ptr) {}

    T* Get() const {
        return ptr_;
    }

    void Release() {
        ptr_ = nullptr;
    }
};

template<typename T>
class Unowned : public IUnowned<T> {
private:
    Handle handle_;

public:
    Unowned(Handle handle) : handle_(handle) {}

    ITempPtr<T> GetTempPtr() const {
        return TempPtr<T>((T*)handle_);
    }
};

template<typename T>
class Owned : public IOwned<T> {
private:
    T* ptr_;
    Handle handle_;

public:
    Owned(T* ptr, Handle handle) :
            ptr_(ptr),
            handle_(handle)
    {}

    IUnowned<T> GetUnowned() const {
        return Unowned<T>(handle_);
    }

    ITempPtr<T> GetTempPtr() const {
        return TempPtr<T>(handle_);
    }

    void Release() {
        if (ptr_ == nullptr) return;
        
        delete ptr_;
        ptr_ = nullptr;
    }
};

}  // namespace htp
