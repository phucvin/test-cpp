#include <iostream>
#include <functional>
#include <vector>
#include <unordered_set>
#include <algorithm>

typedef std::function<void()> Reactor;
typedef int ReactorHandle;

class ReactorStore {
private:
    std::vector<Reactor> reactors_;
    std::unordered_set<ReactorHandle> to_notify_;
    std::unordered_set<ReactorHandle> tmp_to_notify_;

public:
    ReactorHandle Create(Reactor reactor) {
        reactors_.emplace_back(std::move(reactor));
        return reactors_.size() - 1;
    }

    void SetNotify(ReactorHandle handle) {
        to_notify_.insert(handle);
    }

    bool HasNotify() {
        return !to_notify_.empty();
    }

    void Notify() {
        tmp_to_notify_ = std::move(to_notify_);
        std::for_each(tmp_to_notify_.begin(), tmp_to_notify_.end(),
                [this](auto h) { this->reactors_[h](); });
    }

public:
    static ReactorStore& GetSingleton() {
        static ReactorStore instance;
        return instance;
    }
};

class Observable {
private:
    std::vector<ReactorHandle> reactor_handles_;

public:
    void AddReactor(ReactorHandle handle) {
        reactor_handles_.emplace_back(handle);
    }

    void SetNotify() {
        std::for_each(reactor_handles_.begin(), reactor_handles_.end(),
                [](auto h) { ReactorStore::GetSingleton().SetNotify(h); });
    }

    virtual void OnBeforeNotify() = 0;
    virtual void OnAfterNotify() = 0;
};

template<typename T>
class Subject : public Observable {
private:
    T value_;
    T newValue_;
    bool set_;

public:
    explicit Subject(T value) : value_(std::move(value)), set_(false) {}

    void Set(T newValue) {
        if (set_) std::cout << "WARNING: Multiple Set called" << std::endl;

        newValue_ = std::move(newValue);
        SetNotify();
        set_ = true;
    }

    const T& Get() const {
        return value_;
    }

    virtual void OnBeforeNotify() override {
        if (!set_) return;
        value_ = std::move(newValue_);
    }

    virtual void OnAfterNotify() override {
        set_ = false;
    }
};

void Simple01() {
    Subject<int> i1(1);
    Subject<int> i2(2);
    ReactorHandle rh1 = ReactorStore::GetSingleton().Create([&]() {
        std::cout << "react | i1=" << i1.Get() << ", i2=" << i2.Get() << std::endl;
    });
    i1.AddReactor(rh1);
    i2.AddReactor(rh1);
    i1.Set(2);
    i2.Set(3);
    ReactorStore::GetSingleton().Notify();
    i2.Set(4);
    ReactorStore::GetSingleton().Notify();
}

void TwoSum() {
    std::vector<int> v = {1, 2, 3};
    int k = 5;
    Subject<int> left(0);
    Subject<int> right(v.size() - 1);
    Subject<bool> found(false);
    std::vector<Observable*> observables = { &left, &right, &found };

    ReactorHandle rh1 = ReactorStore::GetSingleton().Create([&]() {
        int l = left.Get();
        int r = right.Get();
        std::cout << "tmp01, l=" << l << ", r=" << r << std::endl;
        if (l >= r) return;
        if (v[l] + v[r] < k) {
            left.Set(l+1);
            std::cout << "tmp01, l=" << l+1 << ", r=" << r << std::endl;
        }
    });
    left.AddReactor(rh1);
    found.AddReactor(rh1);
    //
    ReactorHandle rh2 = ReactorStore::GetSingleton().Create([&]() {
        std::cout << "tmp02" << std::endl;
        int l = left.Get();
        int r = right.Get();
        if (l >= r) return;
        if (v[l] + v[r] > k) right.Set(r-1);
    });
    right.AddReactor(rh2);
    found.AddReactor(rh2);
    //
    ReactorHandle rh3 = ReactorStore::GetSingleton().Create([&]() {
        std::cout << "tmp03" << std::endl;
        int l = left.Get();
        int r = right.Get();
        if (v[l] + v[r] == k) found.Set(true);
    });
    found.AddReactor(rh3);
    left.AddReactor(rh3);
    right.AddReactor(rh3);

    found.Set(false);
    while (ReactorStore::GetSingleton().HasNotify()) {
        std::for_each(observables.begin(), observables.end(),
                [](auto* o) { o->OnBeforeNotify(); });
        ReactorStore::GetSingleton().Notify();
        std::for_each(observables.begin(), observables.end(),
                [](auto* o) { o->OnAfterNotify(); });
    }
    std::cout << "TwoSum found=" << std::boolalpha << found.Get() << std::endl;
}

int main() {
    TwoSum();
    return 0;
}