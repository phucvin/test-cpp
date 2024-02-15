#include <iostream>
#include <string>

#include "../hatpop_01.h"

class UserService {
private:
    std::string uname = "user001";

public:
    static htp::Owned<UserService> New() {
        UserService *ptr = new UserService();
        return htp::Owned<UserService>(ptr,
                htp::HandleStore::GetSingleton()->Create(ptr));
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
