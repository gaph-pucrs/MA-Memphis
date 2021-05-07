# Acquiring MIPS Cross-compiler

## Ubuntu (apt-based distros)

For GCC 9.3.0 (latest tested version, recommended) you need to compile it manually:
```console
TODO
```

Otherwise, use the version 4.1.1 bundled with Memphis (you need multilib support):
```console
# dpkg --add-architecture i386
# apt-get update
# apt-get install libc6:i386 libncurses5:i386 libstdc++6:i386
$ mkdir ~/tools_memphis
$ wget https://github.com/GaphGroup/hemps/raw/master/tools/mips-elf-gcc-4.1.1.zip
$ unzip mips-elf-gcc-4.1.1.zip
$ mv mips-elf-gcc-4.1.1 ~/tools_memphis
$ echo -e "# mips-elf-gcc\nPATH=~/tools_memphis/mips-elf-gcc-4.1.1/bin:\${PATH}\nexport MANPATH=~/tools_memphis/mips-elf-gcc-4.1.1/man:\${MANPATH}\n" >> ~/.bashrc
```

## Arch (pacman/aur distros)

Install directly from aur with your prefered helper the package `cross-mips-elf-gcc` or install manually:

```console
# pacman -Syu
$ git clone https://aur.archlinux.org/cross-mips-elf-gcc.git
$ cd cross-mips-elf-gcc
$ makepkg -si
```
