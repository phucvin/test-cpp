#include <atomic>
#include <barrier>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <cassert>

#include "slot_map.h"
#include "haz_ptr.h"
#include "auto_timer.h"
#include "ThreadPool.h"

ENABLE_LOCAL_DOMAIN

typedef dod::slot_map_key64<void*> Handle;

class HandleStore {
private:
    static dod::slot_map64<void*> slot_map_;

public:
    static Handle CreateHandle(void* ptr) {
        return slot_map_.emplace(ptr);
    }

    static void* GetPointerUnsafe(Handle handle) {
        void** pptr = slot_map_.get(handle);
        if (pptr == nullptr || *pptr == nullptr) return nullptr;
        return *pptr;
    }

    static void InvalidateHandle(Handle handle) {
        slot_map_.erase(handle);
    }
};

dod::slot_map64<void*> HandleStore::slot_map_{};

template<typename T>
class TempPtr {
private:
    HazPtrHolder holder_;
    T* ptr_;

public:
    TempPtr(Handle handle) : ptr_(nullptr) {
        ptr_ = holder_.Pin<T>([handle]() {
            return (T*)HandleStore::GetPointerUnsafe(handle);
        });
    }

    // This type is neither moveable nor copyable
    TempPtr(TempPtr&& rhs) = delete;
    TempPtr(const TempPtr&) = delete;

    ~TempPtr() {
        Reset();
    }

    T* Get() const {
        return ptr_;
    }

    T* operator *() const {
        return Get();
    }

    T* operator ->() const {
        return Get();
    }

    void Reset() {
        if (ptr_ == nullptr) return;

        HazPtrRetire(ptr_);
        ptr_ = nullptr;
    }
};

template<typename T>
class Unowned {
private:
    Handle handle_;

public:
    Unowned(Handle handle) : handle_(handle) {}

    TempPtr<T> GetTempPtr() const {
        return TempPtr<T>(handle_);
    }
};

template<typename T>
class Owned {
private:
    T* ptr_;
    Handle handle_;
    TempPtr<T> first_temp_ptr_;

public:
    Owned(T* ptr, Handle handle) :
            ptr_(ptr),
            handle_(handle),
            first_temp_ptr_(handle_)
    {}

    // This type is moveable but not copyable
    Owned(Owned&& rhs) {
        this->ptr_ = rhs.ptr_;
        this->handle_ = rhs.handle_;
        rhs.ptr_ = nullptr;
        rhs.handle_ = {};
    }
    Owned(const Owned&) = delete;

    ~Owned() {
        Reset();
    }

    Handle GetHandle() const {
        return handle_;
    }

    Unowned<T> GetUnowned() {
        return Unowned<T>(handle_);
    }

    T* operator ->() const {
        return ptr_;
    }

    void Reset() {
        if (ptr_ == nullptr) return;

        HandleStore::InvalidateHandle(handle_);
        ptr_ = nullptr;
        handle_ = {};
        first_temp_ptr_.Reset();
    }
};

class UserService {
private:
    std::string host_;

    UserService(const std::string& host) : host_(host) {}

public:
    ~UserService() {
        host_ = "INVALID";
        // std::cout << "~UserService" << std::endl;
    }

    static Owned<UserService> New(const std::string& host) {
        auto* ptr = new UserService(host);
        Handle handle = HandleStore::CreateHandle(ptr);
        return Owned<UserService>(ptr, handle);
    }

    const std::string& GetHost() const { return host_; }
};

class UserPage {
private:
    Unowned<UserService> usrv_;

    UserPage(Unowned<UserService> usrv) : usrv_(usrv) {}

public:
    ~UserPage() {
        // std::cout << "~UserPage" << std::endl;
    }

    static Owned<UserPage> New(Unowned<UserService> usrv) {
        auto* ptr = new UserPage(usrv);
        Handle handle = HandleStore::CreateHandle(ptr);
        return Owned<UserPage>(ptr, handle);
    }

    void Render() {
        TempPtr<UserService> usrv = usrv_.GetTempPtr();
        if (*usrv) {
            auto host = usrv->GetHost();
            assert(host != "INVALID");
            // std::cout << "calling " << host << std::endl;
        } else {
            // std::cout << "skip rendering since UserService is null" << std::endl;
        }
    }
};

void main01(ThreadPool& pool1, ThreadPool& pool2) {
    Owned<UserService> usrv = UserService::New("userservice.api.com");
    Owned<UserPage> upage = UserPage::New(usrv.GetUnowned());
    std::barrier barrier1(2);
    std::barrier barrier2(3);
    pool1.enqueue([&] {
        barrier1.arrive_and_wait();
        upage->Render();
        barrier2.arrive_and_wait();
    });
    pool2.enqueue([&] {
        barrier1.arrive_and_wait();
        usrv.Reset();
        barrier2.arrive_and_wait();
    });
    barrier2.arrive_and_wait();
}

int main() {
    HazPtrInit();
    ThreadPool pool1(1);
    ThreadPool pool2(1);
    {
        AutoTimer timer;
        for (int i = 0; i < 100; ++i) {
            // std::cout << std::endl;
            main01(pool1, pool2);
            // std::cout << std::endl;
        }
    }
    return 0;
}