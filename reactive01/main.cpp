#include <iostream>
#include <functional>
#include <vector>
#include <unordered_set>
#include <algorithm>

typedef std::function<void()> Reactor;
typedef Reactor* ReactorHandle;

class ReactorStore {
private:
    std::vector<Reactor> reactors_;
    std::unordered_set<ReactorHandle> to_notify_;

public:
    ReactorHandle Create(Reactor reactor) {
        reactors_.emplace_back(std::move(reactor));
        return &reactors_[reactors_.size() - 1];
    }

    void SetNotify(ReactorHandle handle) {
        to_notify_.insert(handle);
    }

    void Notify() {
        std::for_each(to_notify_.begin(), to_notify_.end(),
                [](auto h) { (*h)(); });
        to_notify_.clear();
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
};

template<typename T>
class Subject : public Observable {
private:
    T value_;

public:
    explicit Subject(T value) : value_(std::move(value)) {}

    void Set(T newValue) {
        value_ = std::move(newValue);
        SetNotify();
    }

    const T& Get() const {
        return value_;
    }
};

int main() {
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

    return 0;
}