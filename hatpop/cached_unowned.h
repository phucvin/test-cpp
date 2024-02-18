// TODO: duration-based expiration, expire safely async

#include <cassert>

namespace hatp {

template<typename T>
class CachedUnowned;

template<typename T>
class CachedTempPtr {
private:
    CachedUnowned<T>* parent_;
    T* ptr_;

public:
    CachedTempPtr(CachedUnowned<T>* parent, T* ptr)
            : parent_(parent), ptr_(ptr) {}
    ~CachedTempPtr() { if (parent_) parent_->DecAccessCount(); }
    // This type is neither moveable nor copyable
    CachedTempPtr(CachedTempPtr&&) = delete;
    CachedTempPtr(const CachedTempPtr&) = delete;
    T operator *() const { return *Get(); }
    T* operator ->() const { return Get(); }
    operator bool() const { return Get() != nullptr; }
    T* Get() const { return ptr_; }
};

template<typename T>
class CachedUnowned {
private:
    Unowned<T> unowned_;
    TempPtr<T> current_tp_;
    int access_count_;
    int current_access_count_;
    bool accesing_;

    friend class CachedTempPtr<T>;
    void DecAccessCount() {
        assert(accesing_);
        accesing_ = false;
        if (--current_access_count_ <= 0) {
            current_access_count_ = access_count_;
            current_tp_.Release();
        }
    }

public:
    CachedUnowned(Unowned<T> unowned, int access_count)
            : unowned_(unowned), access_count_(access_count),
              current_access_count_(access_count) {}

    CachedTempPtr<T> GetTempPtr() {
        if (accesing_) return CachedTempPtr<T>(nullptr, current_tp_.Get());

        if (current_access_count_ == access_count_ && access_count_ > 0) {
            current_tp_ = unowned_.GetTempPtr();
        }
        accesing_ = true;
        return CachedTempPtr<T>(this, current_tp_.Get());
    }
};

}  // namespace hatp