// TODO: nested accesses don't increase total access count
// TODO: duration-based expiration, expire safely async

namespace hatp {

template<typename T>
class CachedTempPtrBase {
public:
    // This type is neither moveable nor copyable
    // CachedTempPtrBase(CachedTempPtrBase&&) = delete;
    // CachedTempPtrBase(const CachedTempPtrBase&) = delete;

    T operator *() const { return *Get(); }
    T* operator ->() const { return Get(); }
    operator bool() const { return Get() != nullptr; }

    virtual T* Get() const = 0;
};

template<typename T>
class CachedUnowned;

template<typename T>
class CachedTempPtr : public CachedTempPtrBase<T> {
private:
    CachedUnowned<T>& parent_;
    T* ptr_;

public:
    CachedTempPtr(CachedUnowned<T>& parent, T* ptr)
            : parent_(parent), ptr_(ptr) {}

    ~CachedTempPtr() {
        parent_.DecAccessCount();
    }

    virtual T* Get() const { return ptr_; }
};

template<typename T>
class CachedUnowned {
private:
    Unowned<T> unowned_;
    TempPtr<T> current_tp_;
    int access_count_;
    int current_access_count_;

public:
    CachedUnowned(Unowned<T> unowned, int access_count)
            : unowned_(unowned), access_count_(access_count),
              current_access_count_(access_count) {}

    CachedTempPtr<T> GetTempPtr() {
        if (current_access_count_ == access_count_ && access_count_ > 0) {
            current_tp_ = unowned_.GetTempPtr();
        }
        return CachedTempPtr<T>(*this, current_tp_.Get());
    }

    void DecAccessCount() {
        if (--current_access_count_ <= 0) {
            current_access_count_ = access_count_;
            current_tp_.Release();
        }
    }
};

}  // namespace hatp