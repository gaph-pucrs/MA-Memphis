# Acquiring MIPS Cross-compiler

## Ubuntu (apt-based distros)

For GCC 11.1.0 (latest tested version, recommended) you need to compile it manually.
Create a directory for installation and for building:

```console
$ mkdir -p ~/tools_memphis/mips-elf-gcc
$ mkdir ~/mips-cross
```

Obtain binutils, extract, compile and install:
```console
$ cd ~/mips-cross
$ wget https://ftp.gnu.org/gnu/binutils/binutils-2.36.1.tar.xz
$ tar xvf binutils-2.36.1.tar.xz
$ cd binutils-2.36.1
$ mkdir build
$ cd build
$ ../configure --prefix=$HOME/tools_memphis/mips-elf-gcc --program-prefix=mips-elf- --target=mips-elf --disable-werror --disable-nls --with-gcc --with-gnu-as --with-gnu-ld --without-included-gettext
$ make -j `nproc --all`
$ make install
```

Obtain MPC dependency:
```console
# apt-get update
# apt-get install libmpc-dev
```

Obtain GCC, extract, compile and install:
```console
$ cd ~/mips-cross
$ wget ftp://ftp.gnu.org/gnu/gcc/gcc-11.1.0/gcc-11.1.0.tar.xz
$ tar xvf gcc-11.1.0.tar.xz
$ cd gcc-11.1.0
$ mkdir build
$ cd build
$ ../configure --prefix=$HOME/tools_memphis/mips-elf-gcc --program-prefix=mips-elf- --target=mips-elf --oldincludedir=/../../../usr/include --with-gnu-as --with-gnu-ld --disable-nls --disable-threads --enable-languages=c,c++ --disable-multilib --disable-libgcj --enable-lto --disable-werror --without-headers --disable-shared
$ make all-gcc "inhibit_libc=true" -j `nproc --all`
$ make install-gcc
```

Clean-up everything:
```console
$ cd ~
$ rm -rf ~/mips-cross
```

Export the environment variables. Here we do it persistently with .bashrc. Remember to close and reopen the terminal after running:
```console
$ echo -e "# mips-elf-gcc\nPATH=~/tools_memphis/mips-elf-gcc/bin:\${PATH}\nexport MANPATH=~/tools_memphis/mips-elf-gcc/man:\${MANPATH}\n" >> ~/.bashrc
```

**WARNING:** The previous version (4.1.1) bundled with Memphis is not compatible with MA-Memphis.

## Arch (pacman/aur distros)

Install directly from aur with your prefered helper the package `cross-mips-elf-gcc` or install manually.
First, prepare the environment creating a temporary folder and updating the system:
```console
# pacman -Syu
$ mkdir ~/mips-cross
```

Then, install binutils:
```console
$ cd ~/mips-cross
$ git clone https://aur.archlinux.org/cross-mips-elf-binutils.git
$ cd cross-mips-elf-binutils
$ makepkg -si
```

Finally, install GCC:
```console
$ cd ~/mips-cross
$ git clone https://aur.archlinux.org/cross-mips-elf-gcc.git
$ cd cross-mips-elf-gcc
$ makepkg -si
```

Clean-up:
```console
$ cd ~
$ rm -rf ~/mips-cross
```
