```
$ sh <(curl -L https://nixos.org/nix/install) --no-daemon
$ . /home/codespace/.nix-profile/etc/profile.d/nix.sh
$ nix-shell -p clang
$ clang++ -pthread -std=c++20 -o main main.cpp && ./main
```

TODO:
- Use lock-free/wait-free memory reclamation when deleting/retiring a pointer/handle

References:
- https://floooh.github.io/2018/06/17/handles-vs-pointers.html
- https://github.com/SergeyMakeev/slot_map https://docs.rs/slotmap/latest/slotmap/ https://github.com/SilvanVR/SlotMap
- https://github.com/ppetr/lockfree-userspace-rcu
- https://github.com/urcu/userspace-rcu
- https://aturon.github.io/blog/2015/08/27/epoch/
- https://github.com/turingcompl33t/epic (Epoch-based memory reclamation for C++)
- https://github.com/dousbao/cxxhazard
- https://github.com/pramalhe/ConcurrencyFreaks/blob/master/papers/hazarderas-2017.pdf
- https://github.com/topics/hazard-pointer
- https://github.com/huangjiahua/haz_ptr
- https://github.com/bhhbazinga/HazardPointer
- https://github.com/topics/epoch-based-reclamation
- https://github.com/rusnikola/lfsmr (Hyaline Reclamation), https://github.com/ibraheemdev/seize
- https://concurrencyfreaks.blogspot.com/2017/08/why-is-memory-reclamation-so-important.html?m=1
- https://concurrencyfreaks.blogspot.com/2016/08/hazard-pointers-vs-rcu.html
