#pragma once

namespace htp {

// Need to be implemented by the chosen Owned implementation
void* HandleStore_GetSafe(Handle handle);

template<typename T>
class TempPtr {
private:
    Handle handle_;
    T* ptr_;

public:
    TempPtr(Handle handle) : handle_(handle), ptr_(nullptr) {
        ptr_ = (T*)HandleStore_GetSafe(handle);
    }

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
class Unowned {
private:
    Handle handle_;

public:
    Unowned(Handle handle) : handle_(handle) {}

    TempPtr<T> GetTempPtr() const {
        return TempPtr<T>(handle_);
    }
};

}  // namespace htp
