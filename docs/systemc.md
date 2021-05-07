# Acquiring SystemC

## Ubuntu (apt-based distros)

You need to compile it manually:
```console
$ wget https://accellera.org/images/downloads/standards/systemc/systemc-2.3.3.tar.gz
$ tar xvf systemc-2.3.3.tar.gz
$ cd systemc-2.3.3
$ mkdir build
$ cd build
$ mkdir -p ~/tools_memphis/systemc-2.3.3
$ ../configure --prefix=~/tools_memphis/systemc-2.3.3
$ make
$ make install
$ echo -e "# SystemC\nexport SYSTEMC_HOME=~/tools_memphis/systemc-2.3.3\nexport C_INCLUDE_PATH=\${SYSTEMC_HOME}/include\nexport CPLUS_INCLUDE_PATH=\${SYSTEMC_HOME}/include\nexport LIBRARY_PATH=\${SYSTEMC_HOME}/lib-linux64:\${LIBRARY_PATH}\nexport LD_LIBRARY_PATH=\${SYSTEMC_HOME}/lib-linux64:\${LD_LIBRARY_PATH}\n" >> ~/.bashrc
```

## Arch (pacman/aur distros)

SystemC is now available in community packages.
Install:

```console
# pacman -Syu
# pacman -S systemc
```
