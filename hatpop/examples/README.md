```
$ sh <(curl -L https://nixos.org/nix/install) --no-daemon
$ . /home/codespace/.nix-profile/etc/profile.d/nix.sh
$ nix-shell -p clang
$ clang++ -pthread -std=c++20 -o simple01 simple01.cpp && ./simple01
```