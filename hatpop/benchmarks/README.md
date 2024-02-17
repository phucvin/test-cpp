```
$ sh <(curl -L https://nixos.org/nix/install) --no-daemon
$ . /home/codespace/.nix-profile/etc/profile.d/nix.sh
$ nix-shell -p clang
$ clang++ -pthread -std=c++20 -o time01 time01.cpp && ./time01 --confidence=5
$ clang++ -pthread -std=c++20 -o time02 time02.cpp && ./time02 --confidence=5
```

TODO:
- Benchmark time
- Benchmark peak memory usage
- Benchmark high read contention (i.e. a lot of threads reading from the same pointer at the same time)
