#pragma once

namespace htp {

class IHandleStore {
public:
    static IHandleStore* GetSingleton();

    Handle Create(void* ptr);

    void* GetUnsafe(Handle handle);

    void Erase(Handle handle);
};

template<typename T>
class ITempPtr {
public:
    // This type is neither moveable nor copyable
    ITempPtr(ITempPtr&& rhs) = delete;
    ITempPtr(const ITempPtr&) = delete;

    ~ITempPtr() {
        Release();
    }

    T* operator *() const {
        return Get();
    }

    T* operator ->() const {
        return Get();
    }

    T* Get() const;

    void Release();
};

template<typename T>
class IUnowned {
public:
    ITempPtr<T> GetTempPtr() const;
};

template<typename T>
class IOwned {
public:
    // This type is moveable but not copyable
    IOwned(IOwned&& rhs) = default;
    IOwned(const IOwned&) = delete;

    ~IOwned() {
        Release();
    }

    IUnowned<T> GetUnowned() const;

    ITempPtr<T> GetTempPtr() const;

    void Release();
};

}  // namespace htp
