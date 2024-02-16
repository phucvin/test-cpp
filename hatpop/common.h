#pragma once

namespace hatp {

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
    return Owned<T>(new T(std::forward<Args>(args)...));
}

}  // namespace hatp
