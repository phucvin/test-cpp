#include <utility>
#include <iostream>
#include <string>
#include <ios>
#include <cstdint>

template <typename T>
class Handle {
private:
    uint8_t gen_;
    T* ptr_;
    uint8_t* gen_ptr_;
public:
    Handle(uint8_t gen, T* ptr, uint8_t* gen_ptr) : gen_(gen), ptr_(ptr), gen_ptr_(gen_ptr) {}
    uint8_t GetGen() { return gen_; }
    bool Get(T** pptr) const {
        uint8_t ho_gen = *gen_ptr_;
        std::cout << "gen_=" << (int)gen_ << ", ho_gen=" << (int)ho_gen << std::endl;
        if (ho_gen != gen_) return false;
        *pptr = ptr_;
        return true;
    }
};

template <typename T>
class HandledObject {
private:
    uint8_t gen_;
    T obj_;
public:
    template<typename ...Args>
    explicit HandledObject(Args&& ...args) : obj_(std::forward<Args>(args)...) {}
    ~HandledObject() { gen_ += 1; }
    uint8_t GetGen() { return gen_; }
    Handle<T> GetHandle() { return Handle(gen_, &obj_, &gen_); }
};

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
    Handle<UserService> usrv_;

public:
    explicit UserPage(Handle<UserService> usrv) : usrv_(usrv) {}

    void Render() const {
        if (UserService* usrv; usrv_.Get(&usrv)) {
            std::cout << "rendering user page..." << std::endl;
            std::cout << "  user name: " << usrv->GetUserName() << std::endl;
        } else {
            std::cout << "<skip rendering since user service is gone>"
                      << std::endl;
        }
    }
};

int main() {
    auto* usrv = new HandledObject<UserService>("user.api.com");
    auto* upage = new HandledObject<UserPage>(usrv->GetHandle());
    delete usrv;
    if (UserPage* tmp; upage->GetHandle().Get(&tmp)) {
        std::cout << "begin" << std::endl;
        tmp->Render();
    }
    delete upage;
    std::cout << "end" << std::endl;
    return 0;
}