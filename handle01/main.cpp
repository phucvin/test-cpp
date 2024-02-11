#include <iostream>
#include <string>
#include <vector>
#include <memory>

typedef int Handle;

class HandleStore {
private:
    static std::vector<void*> vec;

public:
    static Handle CreateHandle(void* ptr) {
        vec.push_back(ptr);
        return vec.size() - 1;
    }

    template<typename T>
    static Handle CreateHandle(const std::unique_ptr<T>& uptr) {
        return CreateHandle(uptr.get());
    }

    static void* GetPointerUnsafe(Handle handle) {
        return vec[(int)handle];
    }
};

std::vector<void*> HandleStore::vec{};

template<typename T>
class UnownedPtr {
private:
    Handle handle_;

public:
    UnownedPtr(Handle handle) : handle_(handle) {}
};

template<typename T>
class OwnedPtr {
public:
    UnownedPtr<T> GetUnowned() {
        return nullptr;
    }
};

class UserService {
private:
    std::string host_;

public:
    UserService(const std::string& host) : host_(host) {}
    ~UserService() { std::cout << "~UserService" << std::endl; }

    static OwnedPtr<UserService> New() {
        return {};
    }

    const std::string& GetHost() const { return host_; }
};

class HUserService {
private:
    Handle handle_;

public:
    HUserService(Handle handle) : handle_(handle) {}

    const std::string& GetHost() const {
        auto* p = (UserService*)HandleStore::GetPointerUnsafe(handle_);
        return p->GetHost();
    }
};

int main() {
    auto owned_usrv = std::make_unique<UserService>("userservice.api.com");
    HUserService usrv(HandleStore::CreateHandle(owned_usrv));
    std::cout << "hello " << usrv.GetHost() << std::endl;
    return 0;
}