#include <atomic>
#include <barrier>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <cassert>

#include "slot_map.h"

struct ArcRawPtr {
    void* ptr;
    std::atomic_int c;
};

typedef dod::slot_map_key64<ArcRawPtr> Handle;

class HandleStore {
private:
    static dod::slot_map64<ArcRawPtr> slot_map;

public:
    static Handle CreateHandle(void* ptr) {
        return slot_map.emplace(ptr, 0);
    }

    static ArcRawPtr* GetPointerUnsafe(Handle handle) {
        ArcRawPtr* pptr = slot_map.get(handle);
        if (pptr == nullptr) return nullptr;
        return pptr;
    }

    static void InvalidateHandle(Handle handle) {
        slot_map.erase(handle);
    }
};

dod::slot_map64<ArcRawPtr> HandleStore::slot_map{};

template<typename T>
class SnapshotPtr {
private:
    ArcRawPtr* ptr_;

public:
    SnapshotPtr(ArcRawPtr* ptr) : ptr_(ptr) {
        if (ptr_ == nullptr) return;

        ptr_->c.fetch_add(1);
    }

    // This type is neither moveable nor copyable
    SnapshotPtr(SnapshotPtr&& rhs) = delete;
    SnapshotPtr(const SnapshotPtr&) = delete;

    ~SnapshotPtr() {
        Reset();
    }

    T* operator *() const {
        if (ptr_ == nullptr) return nullptr;

        return (T*)ptr_->ptr;
    }

    T* operator ->() const {
        if (ptr_ == nullptr) return nullptr;

        return (T*)ptr_->ptr;
    }

    void Reset() {
        if (ptr_ == nullptr) return;

        if (ptr_->c.fetch_sub(1) == 1) {
            delete (T*)ptr_->ptr;
        }
        ptr_ = nullptr;
    }
};

template<typename T>
class UnownedPtr {
private:
    Handle handle_;

public:
    UnownedPtr(Handle handle) : handle_(handle) {}

    SnapshotPtr<T> Get() const {
        return SnapshotPtr<T>(HandleStore::GetPointerUnsafe(handle_));
    }
};

template<typename T>
class OwnedPtr {
private:
    T* ptr_;
    Handle handle_;
    SnapshotPtr<T> first_snapshot_;

public:
    OwnedPtr(T* ptr, Handle handle) :
            ptr_(ptr),
            handle_(handle),
            first_snapshot_(HandleStore::GetPointerUnsafe(handle_))
    {}

    // This type is moveable but not copyable
    OwnedPtr(OwnedPtr&& rhs) {
        this->ptr_ = rhs.ptr_;
        this->handle_ = rhs.handle_;
        rhs.ptr_ = nullptr;
        rhs.handle_ = {};
    }
    OwnedPtr(const OwnedPtr&) = delete;

    ~OwnedPtr() {
        Reset();
    }

    Handle GetHandle() const {
        return handle_;
    }

    UnownedPtr<T> GetUnowned() {
        return UnownedPtr<T>(handle_);
    }

    T* operator ->() const {
        return ptr_;
    }

    void Reset() {
        if (ptr_ == nullptr) return;

        HandleStore::InvalidateHandle(handle_);
        ptr_ = nullptr;
        handle_ = {};
        first_snapshot_.Reset();
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

    static OwnedPtr<UserService> New(const std::string& host) {
        auto* ptr = new UserService(host);
        Handle handle = HandleStore::CreateHandle(ptr);
        return OwnedPtr<UserService>(ptr, handle);
    }

    const std::string& GetHost() const { return host_; }
};

class UserPage {
private:
    UnownedPtr<UserService> usrv_;

    UserPage(UnownedPtr<UserService> usrv) : usrv_(usrv) {}

public:
    ~UserPage() {
        // std::cout << "~UserPage" << std::endl;
    }

    static OwnedPtr<UserPage> New(UnownedPtr<UserService> usrv) {
        auto* ptr = new UserPage(usrv);
        Handle handle = HandleStore::CreateHandle(ptr);
        return OwnedPtr<UserPage>(ptr, handle);
    }

    const void Render() const {
        SnapshotPtr<UserService> usrv = usrv_.Get();
        if (*usrv) {
            auto host = usrv->GetHost();
            assert(host != "INVALID");
            // std::cout << "calling " << host << std::endl;
        } else {
            // std::cout << "skip rendering since UserService is null" << std::endl;
        }
    }
};

void main01() {
    OwnedPtr<UserService> usrv = UserService::New("userservice.api.com");
    OwnedPtr<UserPage> upage = UserPage::New(usrv.GetUnowned());
    std::barrier barrier(2);
    std::jthread t1([&] {
        barrier.arrive_and_wait();
        upage->Render();
    });
    std::jthread t2([&] {
        barrier.arrive_and_wait();
        usrv.Reset();
    });
}

int main() {
    for (int i = 0; i < 1000; ++i) {
        // std::cout << std::endl;
        main01();
        // std::cout << std::endl;
    }
    return 0;
}