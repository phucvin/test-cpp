#include <iostream>
#include <string>

#include "../hatpop_01.h" // Incorrect, rendering invalid user name
// #include "../hatpop_06.h"  // Deadlock
// #include "../hatpop_07.h"  // OK

class UserService;
htp::Owned<UserService>* _global_usrv;

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
    htp::Unowned<UserService> usrv_;

public:
    explicit UserPage(htp::Unowned<UserService> usrv) : usrv_(usrv) {}

    void Render() const {
        if (auto usrv = usrv_.GetTempPtr(); usrv) {
            std::cout << "rendering user page..." << std::endl;
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
    auto usrv = htp::make_owned<UserService>("user.api.com");
    _global_usrv = &usrv;
    auto upage = htp::make_owned<UserPage>(usrv);
    if (auto tmp = upage.GetTempPtr(); tmp) {
        tmp->Render();
    }
    return 0;
}
