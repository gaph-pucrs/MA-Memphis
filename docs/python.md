# Acquiring Python and needed libraries

## Ubuntu (apt-based distros)

Run:
```console
# apt-get update
# apt-get install python python-yaml
```

## Arch (pacman/aur distros)

Run:
```console
# pacman -Syu
# pacman -S python2 python2-pip
# pip2 install pyyaml
```

## Windows

First download and install Python latest version from [download page](https://www.python.org/downloads/), remembering to check the "Add Python to PATH" option.
Then, open PowerShell and run:
```
pip install pyyaml
```
