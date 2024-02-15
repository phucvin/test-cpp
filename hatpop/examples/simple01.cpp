#include "../hatpop_pointer_pointer.h"

class UserService {
public:
    htp::Owned<UserService> New() {
        UserService *ptr = new UserService();
        return htp::Owned<UserService>(ptr,
                htp::IHandleStore::GetSingleton()->Create(ptr));
    }
};

int main() {
    return 0;
}
