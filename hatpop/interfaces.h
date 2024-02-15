#pragma once

namespace htp {

class IHandleStore {
public:
    static IHandleStore* GetSingleton();

    virtual Handle Create(void* ptr) = 0;

    virtual void* GetUnsafe(Handle handle) = 0;

    virtual void Erase(Handle handle) = 0;
};

template<typename T>
class ITempPtr {
public:
    ITempPtr() = default;
    // TODO: Make this type is neither moveable nor copyable
    /*
    ITempPtr(ITempPtr&& rhs) = delete;
    ITempPtr(const ITempPtr&) = delete;
    */

    ~ITempPtr() {
        Release();
    }

    T* operator *() const {
        return Get();
    }

    T* operator ->() const {
        return Get();
    }

    virtual T* Get() const = 0;

    virtual void Release() = 0;
};

template<typename T>
class IUnowned {
public:
    virtual ITempPtr<T> GetTempPtr() const = 0;
};

template<typename T>
class IOwned {
public:
    IOwned() = default;
    // This type is moveable but not copyable
    IOwned(IOwned&& rhs) = default;
    IOwned(const IOwned&) = delete;

    ~IOwned() {
        Release();
    }

    virtual IUnowned<T> GetUnowned() const = 0;

    virtual ITempPtr<T> GetTempPtr() const = 0;

    virtual void Release() = 0;
};

}  // namespace htp
