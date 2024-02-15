#include <iostream>
#include <string>

#include "../hatpop_02.h"

class UserService {
private:
    std::string uname = "user001";

public:
    static htp::Owned<UserService> New() {
        UserService *ptr = new UserService();
        return htp::Owned<UserService>(ptr,
                htp::HandleStore::GetSingleton()->Create(ptr));
    }

    ~UserService() {
        std::cout << "UserService:dtor" << std::endl;
    }

    std::string GetUserName() const {
        return uname;
    }
};

int main() {
    htp::Owned<UserService> usrv = UserService::New();
    std::cout << usrv.GetTempPtr()->GetUserName() << std::endl;
    return 0;
}
