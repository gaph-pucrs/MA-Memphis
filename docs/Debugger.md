# Acquiring Graphical Debugger

## Pre-requisites

* Java RE (check [how to acquire Java](java.md))

## Acquiring the Debugger

Create a directory to keep the Debugger:
```console
$ mkdir -p ~/tools_memphis/GraphicalDebugger
```

Obtain the Debugger from the github release and move to the created folder:
```console
$ wget https://github.com/gaph-pucrs/GraphicalDebugger/releases/latest/download/Memphis_Debugger.jar
$ mv Memphis_Debugger.jar ~/tools_memphis/GraphicalDebugger
```

Export the environment variables. Here we do it persistently with .bashrc. Remember to close and reopen the terminal after running: 
```console
$ echo -e "# Memphis Debugger\nexport MEMPHIS_DEBUGGER_PATH=~/tools_memphis/GraphicalDebugger\n" >> ~/.bashrc
```
