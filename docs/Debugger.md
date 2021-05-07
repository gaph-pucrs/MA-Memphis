# Acquiring Graphical Debugger

## Pre-requisites

* Java RE (check [how to acquire Java](docs/java.md))

## Acquiring the Debugger

Get it from the github release and keep in the tools folder, then export the environment variable:
```console
$ wget https://github.com/gaph-pucrs/GraphicalDebugger/releases/latest/download/Memphis_Debugger.jar
$ mkdir -p ~/tools_memphis/Debugger
$ mv Memphis_Debugger.jar ~/tools_memphis/Debugger
$ echo -e "# Memphis Debugger\nexport MEMPHIS_DEBUGGER_PATH=~/tools_memphis/Debugger\n" >> ~/.bashrc
```
