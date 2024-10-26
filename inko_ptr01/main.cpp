#include <utility>
#include <iostream>
#include <string>
#include <ios>
#include <cstdint>

template <typename T>
class unowned_ptr;

template <typename T>
class owned_ptr {
public:
    owned_ptr(T* ptr) : ptr_(ptr), refs_(nullptr) {
        if (ptr_ != nullptr) refs_ = new int{0};
    }

    ~owned_ptr() {
    }

    T& operator*() const { return *ptr_; }
    T* operator->() const { return ptr_; }

    void reset() {
        if (ptr_ == nullptr) return;

        if (*refs_ > 0) {
            std::cerr << std::endl;
            std::cerr << "owned_ptr destroying while refs_ > 0" << std::endl;
            exit(723);
        }
        delete ptr_;
        delete refs_;
        ptr_ = nullptr;
        refs_ = nullptr;
    }

private:
    T* ptr_;
    int* refs_;
    friend class unowned_ptr<T>;
};

template <typename T>
class unowned_ptr {
public:
    unowned_ptr(const owned_ptr<T>& owned) : ptr_(owned.ptr_), ref_ptr_(owned.refs_) {
        if (ref_ptr_ != nullptr) *ref_ptr_ += 1;
    }
    unowned_ptr(unowned_ptr<T>&& other) {
        ptr_ = other.ptr_;
        ref_ptr_ = other.ref_ptr_;
        other.ptr_ = nullptr;
        other.ref_ptr_ = nullptr;
    }

    ~unowned_ptr() {
        if (ref_ptr_ != nullptr) *ref_ptr_ -= 1;
    }

    T& operator*() const { return *ptr_; }
    T* operator->() const { return ptr_; }

private:
    T* ptr_;
    int* ref_ptr_;
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
    unowned_ptr<UserService> usrv_;

public:
    explicit UserPage(const owned_ptr<UserService>& usrv) : usrv_(usrv) {}

    void Render() const {
        std::cout << "rendering user page..." << std::endl;
        std::cout << "  user name: " << usrv_->GetUserName() << std::endl;
    }
};

int main() {
    owned_ptr<UserService> usrv(new UserService("user.api.com"));
    owned_ptr<UserPage> upage(new UserPage(usrv));
    // usrv.reset();  // Uncomment this to see error
    upage->Render();
    return 0;
}