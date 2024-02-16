```
$ sh <(curl -L https://nixos.org/nix/install) --no-daemon
$ . /home/codespace/.nix-profile/etc/profile.d/nix.sh
$ nix-shell -p clang
$ clang++ -pthread -std=c++20 -o simple01 simple01.cpp && ./simple01
$ clang++ -pthread -std=c++20 -o race01 race01.cpp && ./race01
$ clang++ -pthread -std=c++20 -o race02 race02.cpp && ./race02
```