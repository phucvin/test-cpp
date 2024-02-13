#include <iostream>
#include <string>
#include <vector>

#include "slot_map.h"

typedef int Handle;

class HandleStore {
private:
    static std::vector<void*> vec;

public:
    static Handle CreateHandle(void* ptr) {
        vec.push_back(ptr);
        return vec.size() - 1;
    }

    static void* GetPointerUnsafe(Handle handle) {
        void* ptr = vec[(int)handle];
        if (ptr == nullptr) ptr = (void*)0xFEFEFEFE;
        return ptr;
    }

    static void InvalidateHandle(Handle handle) {
        vec[(int)handle] = nullptr;
    }
};

std::vector<void*> HandleStore::vec{};

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
        rhs.handle_ = 0;
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
        handle_ = 0;
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