#include <iostream>
#include <string>

// Incorrect, rendering invalid user name
// #include "../hatpop01.h"

// Deadlock
// NOTE: even though it's deadlock, it's very easy to detect and fix this kind
// of deadlock, so it's might be OK to use these in practice due to their
// potential performance advantages over other solutions
// #include "../hatpop04.h"
// #include "../hatpop06.h"

// OK
// #include "../hatpop07.h"
#include "../hatpop09.h"

class UserService;
// NOTE: This is a very bad practice, used here for demo purpose only
// Don't keep pointer to Owned instances, use Unowned or SharedOwned instead
hatp::Owned<UserService>* _global_usrv;

class UserService {
private:
    const std::string host_;
    std::string uname_ = "user001";

public:
    explicit UserService(const std::string& host) : host_(host) {}

    ~UserService() {
        uname_ = "INVALID";
        std::cout << "UserService(host=" << host_ << ")::dtor" << std::endl;
    }

    std::string GetUserName() const {
        return uname_;
    }
};

class UserPage {
private:
    hatp::Unowned<UserService> usrv_;

public:
    explicit UserPage(hatp::Unowned<UserService> usrv) : usrv_(usrv) {}

    void Render() const {
        if (auto usrv = usrv_.GetTempPtr(); usrv) {
            std::cout << "rendering user page..." << std::endl;
            // NOTE: This is a very bad practice, used here for demo purpose only
            // Don't release Owned instance while keeping any TempPtr
            // usrv is still safe and accessible even after releasing
            _global_usrv->Release();
            std::cout << "  user name: " << usrv->GetUserName() << std::endl;
        } else {
            std::cout << "<skip rendering since user service is gone>"
                      << std::endl;
        }
    }
};

int main() {
    auto usrv = hatp::make_owned<UserService>("user.api.com");
    _global_usrv = &usrv;
    auto upage = hatp::make_owned<UserPage>(usrv);
    if (auto tmp = upage.GetTempPtr(); tmp) {
        tmp->Render();
    }
    return 0;
}
