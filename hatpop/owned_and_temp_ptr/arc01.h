#pragma once

#include <atomic>
#include <memory>
#include <cassert>

namespace hatp {

template<typename T>
class CachedTempPtr;
template<typename T>
class CachedUnowned;

template<typename T>
class TempPtr {
private:
    T* ptr_;
    std::shared_ptr<std::atomic_int> arc_;

    // This type is moveable for friends but not copyable for friends/public
    friend class CachedTempPtr<T>;
    friend class CachedUnowned<T>;
    TempPtr(TempPtr&& rhs) {
        this->ptr_ = rhs.ptr_;
        this->arc_ = std::move(rhs.arc_);
        rhs.ptr_ = nullptr;
        rhs.arc_.reset();
    }
    TempPtr& operator=(TempPtr&& rhs) {
        this->ptr_ = rhs.ptr_;
        this->arc_ = std::move(rhs.arc_);
        rhs.ptr_ = nullptr;
        rhs.arc_.reset();
        return *this;
    }
    TempPtr(const TempPtr&) = delete;

public:
    TempPtr() {}
    TempPtr(Handle handle, std::weak_ptr<std::atomic_int> arc)
            : ptr_(nullptr), arc_(arc.lock()) {
        if (arc_ == nullptr) return;
        // The following commented code is optional when weak_ptr is used
        /*
        int curr = arc_->load();
        if (curr <= 0) return;
        */
        if (int prev = arc_->fetch_add(1); prev <= 0) {
            assert(arc_->fetch_sub(1) > 0);
            arc_.reset();
            return;
        }
        ptr_ = (T*)HandleStore::GetSingleton()->GetUnsafe(handle);
        if (ptr_ == nullptr) {
            assert(arc_->fetch_sub(1) > 0);
            arc_.reset();
            return;
        }
    }

    ~TempPtr() { Release(); }
    T operator *() const { return *Get(); }
    T* operator ->() const { return Get(); }
    operator bool() const { return Get() != nullptr; }

    T* Get() const { return ptr_; }

    void Release() {
        if (ptr_ == nullptr) return;

        auto tmp = ptr_;
        ptr_ = nullptr;
        if (arc_->fetch_sub(1) <= 1) delete tmp;
        arc_.reset();
    }
};

template<typename T>
class Unowned {
private:
    Handle handle_;
    std::weak_ptr<std::atomic_int> arc_;

public:
    Unowned(Handle handle, std::weak_ptr<std::atomic_int> arc)
            : handle_(handle), arc_(std::move(arc)) {}

    TempPtr<T> GetTempPtr() const {
        return TempPtr<T>(handle_, arc_);
    }
};

template<typename T>
class Owned {
private:
    T* ptr_;
    Handle handle_;
    std::shared_ptr<std::atomic_int> arc_;

public:
    Owned(T* ptr) : ptr_(ptr), arc_(std::make_shared<std::atomic_int>(1)) {
        if (ptr_) handle_ = HandleStore::GetSingleton()->Create(ptr_);
        else arc_.reset();
    }

    // This type is moveable but not copyable
    Owned(Owned&& rhs) {
        this->ptr_ = rhs.ptr_;
        this->handle_ = rhs.handle_;
        rhs.ptr_ = nullptr;
        rhs.handle_ = {};
    }
    // Common methods & operators
    Owned(const Owned&) = delete;
    ~Owned() { Release(); }
    operator Unowned<T>() { return GetUnowned(); }

    Unowned<T> GetUnowned() const {
        return Unowned<T>(handle_, arc_);
    }

    TempPtr<T> GetTempPtr() const {
        return TempPtr<T>(handle_, arc_);
    }

    void Release() {
        if (ptr_ == nullptr) return;
        
        HandleStore::GetSingleton()->Erase(handle_);
        auto tmp = ptr_;
        ptr_ = nullptr;
        handle_ = {};
        if (arc_->fetch_sub(1) <= 1) delete tmp;
        arc_.reset();
    }
};

}  // namespace hatp
