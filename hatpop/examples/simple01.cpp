#include <iostream>
#include <string>

#include "../hatpop_05.h"

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

int main() {
    auto usrv = htp::make_owned<UserService>("user.api.com");
    std::cout << usrv.GetTempPtr()->GetUserName() << std::endl;
    return 0;
}
