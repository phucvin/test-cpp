#include <iostream>
#include <string>

// #include "../hatpop01.h" // OK only in this simple example, see deadlock01.cpp for when it's incorrect
// #include "../hatpop04.h"  // OK
// #include "../hatpop06.h"  // OK
// #include "../hatpop07.h"  // OK
// #include "../hatpop08.h"  // WIP, ~UserService is not called yet (i.e. leaking)
#include "../hatpop09.h"  // OK
// #include "../hatpop10.h"  // WIP, ~UserService is not called yet (i.e. leaking)

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
            std::cout << "  user name: " << usrv->GetUserName() << std::endl;
        } else {
            std::cout << "<skip rendering since user service is gone>"
                      << std::endl;
        }
    }
};

int main() {
    auto usrv = hatp::make_owned<UserService>("user.api.com");
    auto upage = hatp::make_owned<UserPage>(usrv);
    if (auto tmp = upage.GetTempPtr(); tmp) {
        // usrv.Release();  // Enable this line to see skipped rendering
        tmp->Render();
    }
    return 0;
}
