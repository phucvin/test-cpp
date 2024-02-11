#include <iostream>
#include <string>
#include <vector>

typedef int Handle;

class HandleStore {
private:
    static std::vector<void*> vec;

public:
    static Handle CreateHandle(void* pointer) {
        vec.push_back(pointer);
        return vec.size() - 1;
    }

    static void* GetPointerUnsafe(Handle handle) {
        return vec[(int)handle];
    }
};

std::vector<void*> HandleStore::vec{};

class UserService {
private:
    std::string host_;

public:
    UserService(const std::string& host) : host_(host) {}

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
    HUserService usrv(HandleStore::CreateHandle(new UserService("userservice.api.com")));
    std::cout << "hello " << usrv.GetHost() << std::endl;
    return 0;
}