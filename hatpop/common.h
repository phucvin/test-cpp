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

}  // namespace htp