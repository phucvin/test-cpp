#include <atomic>
#include <barrier>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <cassert>
#include <sys/resource.h>

#include "slot_map.h"
#include "haz_ptr.h"
#include "auto_timer.h"
#include "ThreadPool2.h"

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

public:
    Owned(T* ptr, Handle handle) :
            ptr_(ptr),
            handle_(handle)
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

    TempPtr<T> GetTempPtr() const {
        return TempPtr<T>(handle_);
    }

    void Reset() {
        if (ptr_ == nullptr) return;

        HazPtrRetire(ptr_);
        // delete ptr_;  // Doing this instead of retire will segfault or assert

        HandleStore::InvalidateHandle(handle_);
        ptr_ = nullptr;
        handle_ = {};
    }
};

class UserService {
private:
    int host_;

    UserService(int host) : host_(host) {}

public:
    ~UserService() {
        host_ = -1;
        // std::cout << "~UserService" << std::endl;
    }

    static Owned<UserService> New(int host) {
        auto* ptr = new UserService(host);
        Handle handle = HandleStore::CreateHandle(ptr);
        return Owned<UserService>(ptr, handle);
    }

    int GetHost() const { return host_; }
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
            assert(host != -1);
            // std::cout << "calling " << host << std::endl;
        } else {
            // std::cout << "skip rendering since UserService is null" << std::endl;
        }
    }
};

void main01(ctpl::thread_pool& pool) {
    Owned<UserService> usrv = UserService::New(101);
    Owned<UserPage> upage = UserPage::New(usrv.GetUnowned());
    // std::barrier bar(2);
    std::atomic_int done_count;
    pool.push([&](int) {
        // bar.arrive_and_wait();
        upage.GetTempPtr()->Render();
        done_count.fetch_add(1);
    });
    pool.push([&](int) {
        // bar.arrive_and_wait();
        usrv.Reset();
        done_count.fetch_add(1);
    });
    while (done_count.load() < 2) continue;
}

int main() {
    HazPtrInit();
    ctpl::thread_pool pool(2);
    {
        AutoTimer timer;
        std::atomic_int tmp;
        for (int i = 0; i < 1000000; ++i) {
            tmp.fetch_add(1);
        }
    }
    {
        AutoTimer timer;
        for (int i = 0; i < 1000000; ++i) {
            // std::cout << std::endl;
            main01(pool);
            // std::cout << std::endl;
        }
    }
    {
        struct rusage usage;
        getrusage(RUSAGE_SELF, &usage);
        long max_memory_usage = usage.ru_maxrss;
        // Print the maximum memory usage.
        std::cout << "Maximum memory usage: " << max_memory_usage << " bytes" << std::endl;
    }
    return 0;
}