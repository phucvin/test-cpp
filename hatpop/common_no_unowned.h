#pragma once

namespace htp {

template<typename T>
class Owned;

template<typename T, typename ...Args>
Owned<T> make_owned(Args&& ...args) {
    return Owned<T>(new T(std::forward<Args>(args)...));
}

}  // namespace htp
