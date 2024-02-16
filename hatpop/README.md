# hatpop
<b>H</b>andle <b>A</b>nd <b>T</b>emporary <b>P</b>ointer <b>O</b>ver (raw) <b>P</b>ointer

See `examples` (e.g. `examples\simple01.cpp`) for a quick look first.

Notes:
- Original prototype is in `../handle01`
- `raw_pointer` is unsafe and can result in use-after-free or invalid-memory-access,
unless the same pointer address is never reused for any new objects (i.e. after freeing,
OS can reclaim/reuse the physical memory page contains the pointer address, but
don't reuse that virtual memory page for future allocations)
- `raw_pointer` and `arc` is probably safe, but need more evaluation and testing

References:
- https://floooh.github.io/2018/06/17/handles-vs-pointers.html
- https://github.com/SergeyMakeev/slot_map https://docs.rs/slotmap/latest/slotmap/ https://github.com/SilvanVR/SlotMap
- https://github.com/ppetr/lockfree-userspace-rcu
- https://github.com/urcu/userspace-rcu
- https://concurrencyfreaks.blogspot.com/2016/09/a-simple-userspace-rcu-in-java.html
- https://github.com/pramalhe/ConcurrencyFreaks/blob/master/papers/gracesharingurcu-2017.pdf
- https://aturon.github.io/blog/2015/08/27/epoch/
- https://github.com/turingcompl33t/epic (Epoch-based memory reclamation for C++)
- https://github.com/dousbao/cxxhazard
- https://github.com/pramalhe/ConcurrencyFreaks/blob/master/papers/hazarderas-2017.pdf
- https://github.com/topics/hazard-pointer
- https://github.com/huangjiahua/haz_ptr
- https://bzim.gitlab.io/blog/posts/incinerator-the-aba-problem-and-concurrent-reclamation.html
- https://ticki.github.io/blog/fearless-concurrency-with-hazard-pointers/
- https://github.com/bhhbazinga/HazardPointer
- https://github.com/topics/epoch-based-reclamation
- https://github.com/rusnikola/lfsmr (Hyaline Reclamation), https://github.com/ibraheemdev/seize
- https://concurrencyfreaks.blogspot.com/2017/08/why-is-memory-reclamation-so-important.html?m=1
- https://concurrencyfreaks.blogspot.com/2016/08/hazard-pointers-vs-rcu.html
