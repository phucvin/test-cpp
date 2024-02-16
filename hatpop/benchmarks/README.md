```
$ sh <(curl -L https://nixos.org/nix/install) --no-daemon
$ . /home/codespace/.nix-profile/etc/profile.d/nix.sh
$ nix-shell -p clang
$ TODO
```

TODO:
- Benchmark time
- Benchmark peak memory usage
- Benchmark high read contention (i.e. a lot of threads reading from the same pointer at the same time)
