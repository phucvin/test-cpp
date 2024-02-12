#include <iostream>
#include <string>
#include <vector>

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
    auto owned_usrv = UserService::New("userservice.api.com");
    owned_usrv.Reset();
    UnownedPtr<UserService> usrv = owned_usrv.GetUnowned();
    std::cout << "hello " << usrv->GetHost() << std::endl;
    return 0;
}