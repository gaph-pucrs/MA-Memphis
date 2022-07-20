# Acquiring SystemC

## Ubuntu (apt-based distros)

You need to compile it manually. Create a temporary folder and a destination folder:
```console
mkdir ~/systemc
mkdir -p ~/tools_memphis/systemc
```

Obtain the source, extract, compile and install:
```console
cd ~/systemc
wget https://accellera.org/images/downloads/standards/systemc/systemc-2.3.3.tar.gz
tar xf systemc-2.3.3.tar.gz
cd systemc-2.3.3
mkdir build
cd build
../configure --prefix=$HOME/tools_memphis/systemc --enable-debug
make -j `nproc --all`
make install
```

Clean-up:
```console
cd ~
rm -rf ~/systemc
```

Export the PKGCONFIG and LD_LIBRARY_PATH environment variables. Here we do it persistently with .bashrc. Remember to close and reopen the terminal after running: 
```console
echo -e "# SystemC\nexport SYSTEMC_ROOT=~/tools_memphis/systemc\nPKG_CONFIG_PATH=$SYSTEMC_ROOT/lib-linux64/pkgconfig:$PKG_CONFIG_PATH\nLD_LIBRARY_PATH=$SYSTEMC_ROOT/lib-linux64:$LD_LIBRARY_PATH" >> ~/.bashrc
```

## Arch (pacman/aur distros)

SystemC is now available in community packages.
Install:

```console
pacman -Syu
pacman -S systemc
```
