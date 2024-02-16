#pragma once

namespace hatp {

template<typename T>
class Owned;

template<typename T, typename ...Args>
Owned<T> make_owned(Args&& ...args) {
    return Owned<T>(new T(std::forward<Args>(args)...));
}

}  // namespace hatp
