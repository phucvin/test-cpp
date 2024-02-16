#include <iostream>
#include <string>

#include "../hatpop_06.h"

class UserService {
private:
    const std::string host_;
    std::string uname_ = "user001";

public:
    explicit UserService(const std::string& host) : host_(host) {}

    ~UserService() {
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
            std::cout << usrv->GetUserName() << std::endl;
        } else {
            std::cout << "<skip rendering since user service is gone>"
                      << std::endl;
        }
    }
};

int main() {
    auto usrv = htp::make_owned<UserService>("user.api.com");
    auto upage = htp::make_owned<UserPage>(usrv.GetUnowned());
    if (auto tmp = upage.GetTempPtr(); tmp) {
        tmp->Render();
    }
    return 0;
}
