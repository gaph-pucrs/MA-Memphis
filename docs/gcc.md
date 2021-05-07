# Acquiring GCC (build tools)

## Ubuntu (apt-based distros)

Run (WARNING: maybe its missing rsync and bsdmainutils (for hexdump)):
```console
# apt-get update
# apt-get install build-essential
```

## Arch (pacman/aur distros)

Run:
```console
# pacman -Syu
# pacman -S base-devel rsync coreutils --needed
```
