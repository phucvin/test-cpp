#include <iostream>
#include <string>
#include <vector>
#include <cassert>

#include "slot_map.h"

typedef dod::slot_map_key64<void*> Handle;

class HandleStore {
private:
    static dod::slot_map64<void*> slot_map;

public:
    static Handle CreateHandle(void* ptr) {
        return slot_map.emplace(ptr);
    }

    static void* GetPointerUnsafe(Handle handle) {
        void** pptr = slot_map.get(handle);
        assert(pptr != nullptr);
        return *pptr;
    }

    static void InvalidateHandle(Handle handle) {
        slot_map.erase(handle);
    }
};

dod::slot_map64<void*> HandleStore::slot_map{};

template<typename T>
class UnownedPtr {
private:
    Handle handle_;

public:
    UnownedPtr(Handle handle) : handle_(handle) {}

    T* operator ->() {
        return (T*)HandleStore::GetPointerUnsafe(handle_);
    }
};

template<typename T>
class OwnedPtr {
private:
    T* ptr_;
    Handle handle_;

public:
    OwnedPtr(T* ptr, Handle handle) : ptr_(ptr), handle_(handle) {}

    // This type is moveable but not copyable
    OwnedPtr(OwnedPtr&& rhs) {
        this->ptr_ = rhs.ptr_;
        this->handle_ = rhs.handle_;
        rhs.ptr_ = nullptr;
        rhs.handle_ = {};
    }
    OwnedPtr(const OwnedPtr&) = delete;

    ~OwnedPtr() {
        Reset();
    }

    UnownedPtr<T> GetUnowned() {
        return UnownedPtr<T>(handle_);
    }

    T* operator ->() {
        return ptr_;
    }

    void Reset() {
        if (ptr_ == nullptr) return;

        HandleStore::InvalidateHandle(handle_);
        delete ptr_;
        ptr_ = nullptr;
        handle_ = {};
    }
};

class UserService {
private:
    std::string host_;

    UserService(const std::string& host) : host_(host) {}

public:
    ~UserService() { std::cout << "~UserService" << std::endl; }

    static OwnedPtr<UserService> New(const std::string& host) {
        auto* ptr = new UserService(host);
        Handle handle = HandleStore::CreateHandle(ptr);
        return OwnedPtr<UserService>(ptr, handle);
    }

    const std::string& GetHost() const { return host_; }
};

int main() {
    auto tmp = UserService::New("userservice.api.com");
    auto owned_usrv = std::move(tmp);
    UnownedPtr<UserService> usrv = owned_usrv.GetUnowned();
    std::cout << "hello " << usrv->GetHost() << std::endl;
    std::cout << "bye " << owned_usrv->GetHost() << std::endl;
    owned_usrv.Reset();
    return 0;
}