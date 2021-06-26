# Acquiring SystemC

## Ubuntu (apt-based distros)

Install the cmake pre-requisite:
```console
# apt update
# apt install cmake
```

You need to compile it manually. Create a temporary folder and a destination folder:
```console
$ mkdir ~/systemc
$ mkdir -p ~/tools_memphis/systemc
```

Obtain the source, extract, compile and install:
```console
$ cd ~/systemc
$ wget https://accellera.org/images/downloads/standards/systemc/systemc-2.3.3.tar.gz
$ tar xf systemc-2.3.3.tar.gz
$ cd systemc-2.3.3
$ mkdir build
$ cd build
$ cmake .. -DCMAKE_CXX_STANDARD=11 -DCMAKE_INSTALL_PREFIX=$HOME/tools_memphis/systemc
$ make -j `nproc --all`
$ make install
```

Clean-up:
```console
$ cd ~
$ rm -rf ~/systemc
```

Export the environment variables:
* C_INCLUDE_PATH
* CPLUS_INCLUDE_PATH
* LIBRARY_PATH
* LD_LIBRARY_PATH
  
Here we do it persistently with .bashrc. Remember to close and reopen the terminal after running: 
```console
$ echo -e "# SystemC\nexport SYSTEMC_HOME=~/tools_memphis/systemc\nexport C_INCLUDE_PATH=\${SYSTEMC_HOME}/include\nexport CPLUS_INCLUDE_PATH=\${SYSTEMC_HOME}/include\nexport LIBRARY_PATH=\${SYSTEMC_HOME}/lib-linux64:\${LIBRARY_PATH}\nexport LD_LIBRARY_PATH=\${SYSTEMC_HOME}/lib-linux64:\${LD_LIBRARY_PATH}\n" >> ~/.bashrc
```

## Arch (pacman/aur distros)

SystemC is now available in community packages.
Install:

```console
# pacman -Syu
# pacman -S systemc
```
