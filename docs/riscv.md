# Acquiring RISCV Cross-compiler

## Ubuntu (apt-based distros)

For GCC 11.3.0 (latest tested version, recommended) you need to compile it manually.
Create a directory for installation and for building:

```console
$ mkdir -p ~/tools_memphis/riscv-elf
$ mkdir ~/riscv-elf
```

Obtain binutils, extract, compile and install:
```console
$ cd ~/riscv-elf
$ wget https://ftp.gnu.org/gnu/binutils/binutils-2.38.tar.xz
$ tar xvf binutils-2.38.tar.xz
$ cd binutils-2.38
$ sed -i '/ac_cpp=/s/\$CPPFLAGS/\$CPPFLAGS -O2/' libiberty/configure
$ mkdir build
$ cd build
$ ../configure --prefix=$HOME/tools_memphis/riscv-elf --target=riscv64-elf --with-sysroot=$HOME/tools_memphis/riscv-elf/riscv-elf --enable-interwork --enable-multilib --with-gnu-as --with-gnu-ld --disable-nls --enable-ld=default --enable-gold --enable-plugins --enable-ld=default --enable-deterministic-archives
$ make LDFLAGS="" -j$(nproc)
$ make install
```

Obtain newlib:
```console
$ cd ~/riscv-elf
$ wget https://sourceware.org/pub/newlib/newlib-4.2.0.20211231.tar.gz
$ tar xvf newlib-4.2.0.20211231.tar.gz
```

Obtain GCC, extract, compile and install:
```console
$ cd ~/riscv-elf
$ wget ftp://ftp.gnu.org/gnu/gcc/gcc-11.3.0/gcc-11.3.0.tar.xz
$ tar xvf gcc-11.3.0.tar.xz
$ cd gcc-11.3.0
$ echo $_gccver > gcc/BASE-VER
$ contrib/download_prerequisites
$ sed -i "/ac_cpp=/s/\$CPPFLAGS/\$CPPFLAGS -O2/" {libiberty,gcc}/configure
$ mkdir build
$ cd build
$ export CFLAGS_FOR_TARGET='-g -Os -ffunction-sections -fdata-sections'
$ export CXXFLAGS_FOR_TARGET='-g -Os -ffunction-sections -fdata-sections'
$ ../configure --prefix=$HOME/tools_memphis/riscv-elf --target=riscv64-elf --with-sysroot=$HOME/tools_memphis/riscv-elf/riscv-elf --with-native-system-header-dir=/include --enable-languages=c,c++ --enable-threads=single --enable-plugins --enable-multilib --enable-libgcc --disable-libgomp --disable-libquadmath --disable-libffi --disable-libssp --disable-libmudflap --disable-decimal-float --disable-libstdcxx-pch --disable-nls --disable-shared --disable-tls --with-newlib --with-gnu-as --with-gnu-ld --with-system-zlib --with-headers=../../newlib-4.2.0.20211231/newlib/libc/include --with-libelf --enable-gnu-indirect-function
$ make -j$(nproc)
$ make install
```

Build newlib and install:
```console
$ export PATH=$HOME/tools_memphis/riscv-elf/bin:$PATH
$ cd ~/riscv-elf/newlib-4.2.0.20211231
$ mkdir build
$ cd build
$ ../configure --target=riscv64-elf --prefix=$HOME/tools_memphis/riscv-elf --enable-newlib-io-long-long --enable-newlib-io-c99-formats --enable-newlib-register-fini --enable-newlib-retargetable-locking --disable-newlib-supplied-syscalls --disable-nls
make -j$(nproc)
make install
```

Clean-up everything:
```console
$ cd ~
$ rm -rf ~/riscv-elf
```

Export the environment variables:
* PATH
* MANPATH
   
Here we do it persistently with .bashrc. Remember to close and reopen the terminal after running:
```console
$ echo -e "# riscv-elf\nPATH=~/tools_memphis/riscv-elf/bin:\${PATH}\nexport MANPATH=~/tools_memphis/riscv-elf/man:\${MANPATH}\n" >> ~/.bashrc
```

## Arch (pacman/aur distros)

Install directly from pacman:
```console
# pacman -Syu
# pacman -S riscv64-elf-gcc riscv64-elf-binutils riscv64-elf-newlib
```
