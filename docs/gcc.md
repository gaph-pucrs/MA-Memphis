# Acquiring GCC (build tools)

## Ubuntu (apt-based distros)

Install the compiler tools (build-essential) and git, 
```console
# apt-get update
# apt-get install build-essential git
```

## Arch (pacman/aur distros)

Run:
```console
# pacman -Syu
# pacman -S base-devel rsync coreutils --needed
```
