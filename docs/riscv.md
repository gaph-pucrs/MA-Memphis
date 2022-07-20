# Acquiring RISCV Cross-compiler

## CentOS (yum/dnf-based distros) and Ubuntu (apt-based distros)

For GCC 12.1.0 (latest tested version, recommended) you need to build it manually.
Create a directory for installation and for building:

```console
mkdir -p ~/tools_memphis/riscv64-elf
mkdir ~/riscv64-elf
```

Obtain binutils, extract, compile and install:
```console
cd ~/riscv64-elf
wget https://ftp.gnu.org/gnu/binutils/binutils-2.38.tar.xz
tar xf binutils-2.38.tar.xz
cd binutils-2.38
sed -i '/ac_cpp=/s/\$CPPFLAGS/\$CPPFLAGS -O2/' libiberty/configure
mkdir build
cd build
../configure --prefix=$HOME/tools_memphis/riscv64-elf --target=riscv64-elf --with-sysroot=$HOME/tools_memphis/riscv64-elf/riscv64-elf --enable-interwork --enable-multilib --with-gnu-as --with-gnu-ld --disable-nls --enable-ld=default --enable-gold --enable-plugins --enable-ld=default --enable-deterministic-archives
make -j$(nproc)
make install
rm $HOME/tools_memphis/riscv64-elf/share/man/man1/riscv64-elf-{dlltool,windres,windmc}*
rm $HOME/tools_memphis/riscv64-elf/lib/bfd-plugins/libdep.so
rm -r $HOME/tools_memphis/riscv64-elf/share/info
```

Obtain newlib and extract to build GCC:
```console
cd ~/riscv64-elf
wget https://sourceware.org/pub/newlib/newlib-4.2.0.20211231.tar.gz
tar xf newlib-4.2.0.20211231.tar.gz
```

Obtain GCC, extract, build and install:
```console
cd ~/riscv64-elf
wget ftp://ftp.gnu.org/gnu/gcc/gcc-12.1.0/gcc-12.1.0.tar.xz
tar xvf gcc-12.1.0.tar.xz
cd gcc-12.1.0
echo 12.1.0 > gcc/BASE-VER
sed -i "/ac_cpp=/s/\$CPPFLAGS/\$CPPFLAGS -O2/" {libiberty,gcc}/configure
wget https://gcc.gnu.org/pub/gcc/infrastructure/isl-0.24.tar.bz2
tar xf isl-0.24.tar.bz2
ln -s isl-0.24.tar.bz2 isl
mkdir build
cd build
export CFLAGS_FOR_TARGET='-g -Os -ffunction-sections -fdata-sections'
export CXXFLAGS_FOR_TARGET='-g -Os -ffunction-sections -fdata-sections'
CFLAGS=${CFLAGS/-Werror=format-security/} CXXFLAGS=${CXXFLAGS/-Werror=format-security/} ../configure --prefix=$HOME/tools_memphis/riscv64-elf --target=riscv64-elf --with-sysroot=$HOME/tools_memphis/riscv64-elf/riscv64-elf --with-native-system-header-dir=/include --enable-languages=c,c++ --enable-threads=single --enable-plugins --enable-multilib --enable-libgcc --disable-libgomp --disable-libquadmath --disable-libffi --disable-libssp --disable-libmudflap --disable-decimal-float --disable-libstdcxx-pch --disable-nls --disable-shared --disable-tls --with-newlib --with-gnu-as --with-gnu-ld --with-system-zlib --with-headers=../../newlib-4.2.0.20211231/newlib/libc/include --with-python-dir=share/gcc-riscv64-elf --with-gmp --with-mpfr --with-mpc --with-isl --with-libelf --enable-gnu-indirect-function
make -j$(nproc)
make install
find $HOME/tools_memphis/riscv64-elf/lib/gcc/riscv64-elf/12.1.0 $HOME/tools_memphis/riscv64-elf/riscv64-elf/lib \
    -type f -and \( -name \*.a -or -name \*.o \) \
    -exec $_target-objcopy -R .comment -R .note -R .debug_info -R .debug_aranges \
    -R .debug_pubnames -R .debug_pubtypes -R .debug_abbrev -R .debug_line \
    -R .debug_str -R .debug_ranges -R .debug_loc '{}' \;
find $HOME/tools_memphis/riscv64-elf/bin/ $HOME/tools_memphis/riscv64-elf/lib/gcc/riscv64-elf/12.1.0 -type f -and \( -executable \) -exec strip '{}' \;
rm -r $HOME/tools_memphis/riscv64-elf/share/man/man7
rm -r $HOME/tools_memphis/riscv64-elf/share/info
rm $HOME/tools_memphis/riscv64-elf/lib64/libcc1.*
```

Build newlib-nano and install:
```console
export PATH=$HOME/tools_memphis/riscv64-elf/bin:$PATH
cd ~/riscv64-elf/newlib-4.2.0.20211231
mkdir build-nano
cd build-nano
export CFLAGS_FOR_TARGET='-g -Os -ffunction-sections -fdata-sections'
../configure --target=riscv64-elf --prefix=$HOME/tools_memphis/riscv64-elf --enable-newlib-reent-small --enable-newlib-retargetable-locking --enable-newlib-nano-malloc --enable-lite-exit --enable-newlib-global-atexit --enable-newlib-nano-formatted-io --disable-newlib-fvwrite-in-streamio --disable-newlib-fseek-optimization --disable-newlib-wide-orient --disable-newlib-unbuf-stream-opt --disable-newlib-supplied-syscalls --disable-nls
make -j$(nproc)
make install
find "$HOME/tools_memphis/riscv64-elf/riscv64-elf" -regex ".*/lib\(c\|g\|m\|rdimon\|gloss\)\.a" -exec rename .a _nano.a '{}' \;
install -d "$HOME/tools_memphis/riscv64-elf"/riscv64-elf/include/newlib-nano
install -m644 -t "$HOME/tools_memphis/riscv64-elf"/riscv64-elf/include/newlib-nano "$HOME/tools_memphis/riscv64-elf"/riscv64-elf/include/newlib.h
```

Build newlib and install:
```console
cd ~/riscv64-elf/newlib-4.2.0.20211231
mkdir build-newlib
cd build-newlib
export CFLAGS_FOR_TARGET='-g -O2 -ffunction-sections -fdata-sections'
../configure --target=riscv64-elf --prefix=$HOME/tools_memphis/riscv64-elf --enable-newlib-io-long-long --enable-newlib-io-c99-formats --enable-newlib-register-fini --enable-newlib-retargetable-locking --disable-newlib-supplied-syscalls --disable-nls
make -j$(nproc)
make install
find "$HOME/tools_memphis/riscv64-elf "/riscv64-elf/lib \( -name "*.a" -or -name "*.o" \) \
       -exec riscv64-elf-objcopy -R .comment -R .note -R .debug_info -R .debug_aranges \
       -R .debug_pubnames -R .debug_pubtypes -R .debug_abbrev -R .debug_line \
       -R .debug_str -R .debug_ranges -R .debug_loc '{}' \;
cd ../
install -d $HOME/tools_memphis/riscv64-elf/share/licenses/riscv64-elf-newlib/
install -m644 -t $HOME/tools_memphis/riscv64-elf/share/licenses/riscv64-elf-newlib/ ./COPYING*
```

Clean-up everything:
```console
cd ~
rm -rf ~/riscv64-elf
```

Export the environment variables:
* PATH
* MANPATH
   
Here we do it persistently with .bashrc. Remember to close and reopen the terminal after running:
```console
echo -e "# riscv-elf\nPATH=~/tools_memphis/riscv-elf/bin:\${PATH}\nexport MANPATH=~/tools_memphis/riscv-elf/man:\${MANPATH}\n" >> ~/.bashrc
```

## Arch (pacman/aur distros)

Install directly from pacman:
```console
pacman -Syu
pacman -S riscv64-elf-gcc riscv64-elf-binutils riscv64-elf-newlib
```
