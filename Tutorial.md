# Memphis - Usage @Gaph

## Obtain files

First, get Memphis from github:
```console
$ git clone https://github.com/gaph-pucrs/MA-Memphis
```

## Configure the environment

Add MA_MEMPHIS_PATH and Memphis builders to the PATH:
```console
$ echo -e "# Memphis\nexport MA_MEMPHIS_PATH=~/MA-Memphis\nexport PATH=\${MA_MEMPHIS_PATH}/bin:\${PATH}\n" >> ~/.bashrc
```

Then exit and restart the SSH session.

## Load dependencies

Load SystemC:
```console
$ module load systemc
```

Load compatible GCC:
```
$ source /opt/rh/devtoolset-11/enable
```

Load RISC-V cross-compiler:
```console
$ module load riscv64-elf
```

## Testing

Create a sandbox folder:
```
$ mkdir ~/sandbox
$ cd ~/sandbox
```

Create `my_testcase.yaml` inside the sandbox folder with the following content:
```yaml
hw:
  page_size_KB: 64
  tasks_per_PE: 2
  mpsoc_dimension: [3,3]
  Peripherals:              # Attached peripherals
    - name: APP_INJECTOR    # Mandatory Application Injector peripheral
      pe: 2,2               # is at border PE 2,2
      port: N               # Connected at port NORTH. Note to use a border port.
    - name: MAINJECTOR      # Mandatory MA Injector peripheral
      pe: 0,0               # is connected at border PE 0,0
      port: S               # Connected at port SOUTH. Note to use a border port.
```

Create `my_scenario.yaml`inside the sandbox folder with the following content:
```yaml
management:                 # Management application properties
  - task: mapper_task       # The first in this list should ALWAYS be mapper_task
    static_mapping: [0,0]   # All management task should have static mapping defined
apps:
  - name: mpeg
  - name: dijkstra
    static_mapping:
      divider: [0,1]
```

Inside the sandbox folder, generate the testcase:
```console
$ memphis testcase my_testcase.yaml
```

Now, generate the scenario:
```console
$ memphis scenario my_testcase my_scenario.yaml
```

To simulate, run:
```console
$ memphis simulate my_testcase/my_scenario -t 50
```

Simulation should start logging stdout and the Graphical Debugger should open.
