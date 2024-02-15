#include <iostream>
#include <string>

#include "../hatpop_01.h"

class UserService {
public:
    static htp::Owned<UserService> New() {
        UserService *ptr = new UserService();
        return htp::Owned<UserService>(ptr,
                htp::HandleStore::GetSingleton()->Create(ptr));
    }

    std::string GetUserName() {
        return "uname";
    }
};

int main() {
    htp::Owned<UserService> usrv = UserService::New();
    std::cout << usrv.GetTempPtr()->GetUserName() << std::endl;
    return 0;
}
