#pragma once

namespace htp {

template<typename T>
class TempPtr;

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

template<typename T>
class Owned;

template<typename T, typename ...Args>
Owned<T> make_owned(Args&& ...args) {
    T* ptr = new T(std::forward<Args>(args)...);
    Handle handle = HandleStore::GetSingleton()->Create(ptr);
    return Owned<T>(ptr, handle);
}

}  // namespace htp
