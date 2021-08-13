# nRF Development in Linux

If you need/want to have nRF development environment in Linux...

<br/><br/>

## Environment

- Ubuntu 21.04 x86_64 
- nRF52832 DK board

<br/><br/>

## Index

1. Getting the SDK and tools
   - 1.1 Get basic tools
   - 1.2 Get ARM-GCC Compiler
   - 1.3 Get nRF SDK and docs
   - 1.4 Get command line JTAG tools
   - 1.5 Flash SoftDevice
2. Building the example
   - 2.1 Modify Makefile.Posix for SDK
   - 2.2 Compile an Example
   - 2.3 Simplify the example
   - 2.4 Edit Makefile for the simplified project
3. Setting VSCODE as the IDE 
   - 3.1 Get VSCODE and some extensions
   - 3.2 Set Tasks.json for make/clean  
   - 3.3 Set Launch.json for debugging
   - 3.4 Get Python tools
4. Conclusion

<br/><br/>

## 1. Getting the SDK and tools

<br/>

### 1.1 Get Basic Tools

Install some packages:
```sh
$ sudo apt update
$ sudo apt install -y \
    git \
    libncurses5 \
    gdb-multiarch \
    build-essential
```

If libncurses5 installation fails:
```
$ sudo apt --fix-broken install
```

<br/><br/>

### 1.2 Get ARM-GCC Compiler

ARM provides ARM-GCC compiler
- [Downloads](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)

To install the stable version:
```sh
# Prep the directory
$ mkdir -p ~/nRF52
$ cd ~/nRF52

# Download and extract
$ wget https://developer.arm.com/-/media/Files/downloads/gnu-rm/10.3-2021.07/gcc-arm-none-eabi-10.3-2021.07-x86_64-linux.tar.bz2
$ tar jxf gcc-arm-none-eabi-10.3-2021.07-x86_64-linux.tar.bz2 # take ~30 Sec.
$ mv gcc-arm-none-eabi-10.3-2021.07 gcc-arm 

# Check
$ ./gcc-arm/bin/arm-none-eabi-gcc -v
...
gcc version 10.3.1 20210621 (release) (GNU Arm Embedded Toolchain 10.3-2021.07) 
```

Don't forget adding **~/nRF52/gcc-arm/bin** to your PATH!!
```sh
$ echo "export PATH=$PATH:$HOME/nRF52/gcc-arm/bin" >> ~/.bashrc
```

<br/><br/>

### 1.3 Get nRF52 SDK and documents

Download the SDK and install:
```sh
$ cd ~/nRF52
$ wget https://developer.nordicsemi.com/nRF51_SDK/nRF5_SDK_v17.x.x/nRF5_SDK_17.0.2_d674dde.zip
$ unzip nRF5_SDK_17*.zip
$ mv nRF5_SDK_17.0.2_d674dde nRF5_SDK
```

Downloads the documents and install:
```sh
$ cd ~/nRF52
$ wget https://developer.nordicsemi.com/nRF51_SDK/nRF5_SDK_v17.x.x/nRF5_SDK_17.0.2_offline_doc.zip
$ unzip nRF5_*doc.zip -d docs
```

<br/><br/>

### 1.4 Get command line JTAG tools

Download **nRF5x-Command-Line-Tools** for Linux 64 bit from:
- [https://www.nordicsemi.com](https://www.nordicsemi.com/Software-and-tools/Development-Tools/nRF-Command-Line-Tools/Download#infotabs)

Run these commands to install the tools:
```sh
$ cd ~/nRF52
$ wget https://www.nordicsemi.com/-/media/Software-and-other-downloads/Desktop-software/nRF-command-line-tools/sw/Versions-10-x-x/10-13-0/nRF-Command-Line-Tools_10_13_0_Linux64.zip
$ unzip nRF-Command-Line-Tools_10_13_0_Linux-amd64.zip -d tools

$ cd tools/nRF-Command-Line-Tools_10_13_0_Linux64
$ tar xvf nRF-Command-Line-Tools_10_13_0_Linux-amd64.tar.gz 
$ sudo dpkg -i JLink_Linux_V750b_x86_64.deb
$ sudo dpkg -i nRF-Command-Line-Tools_10_13_0_Linux-amd64.deb
```

To check the version of installed tools:
```sh
$ nrfjprog -v
nrfjprog version: 10.13.0 
JLinkARM.dll version: 7.50a

$ mergehex -v
mergehex version: 10.13.0

$ JLinkExe -v
SEGGER J-Link Commander V7.50a (Compiled Jul  8 2021 18:21:10)
DLL version V7.50a, compiled Jul  8 2021 18:20:53
```

<br/><br/>

### 1.5 Flash SoftDevice

To flash a softdevice to a connected nRF52832-DK (s132) board via JLink,
```sh
$ cd ~/nRF52/nRF52_SDK/components/softdevice/s132/hex
$ ls -l
s132_nrf52_7.2.0_licence-agreement.txt
s132_nrf52_7.2.0_softdevice.hex

$ nrfjprog -f NRF52 --chiperase --program s132_nrf52_7.2.0_softdevice.hex 
```

<br/><br/>

## 2. Building the example
  
<br/>

### 2.1 Modify Makefile.Posix for SDK

Get the information of the installed compiler:
```sh
$ which arm-none-eabi-gcc
$HOME/nRF52/gcc-arm/bin/arm-none-eabi-gcc

$ arm-none-eabi-gcc -v
...
gcc version 10.3.1 20210621 (release) (GNU Arm Embedded Toolchain 10.3-2021.07) 
```

Edit Makefile:
```sh
$ cd ~/nRF52/nRF52_SDK/components/toolchain/gcc
$ vi Makefile.posix
```

Then edit the Makefile looks like based on the information we got:
```Makefile
GNU_INSTALL_ROOT ?= ${HOME}/nRF52/gcc-arm/bin/
GNU_VERSION ?= 10.3.1
GNU_PREFIX ?= arm-none-eabi
```

<br/><br/>

### 2.2 Compile an Example

Now we can just compile an example:
```sh
$ cd ~/nRF52/nRF52_SDK/examples/peripheral/blinky/pca10040/s132/armgcc
$ ls -l
blinky_gcc_nrf52.ld 
Makefile

$ make
...
DONE nrf52832_xxaa
```

<br/><br/>

### 2.3 Simplify the example

nRF52 SDK comes with well organized files, but sometimes simplifying project hierarchy might be a good idea. 
```
$ cd $HOME/nRF52/nRF5_SDK/examples/peripheral/blinky
$ tree 
.
├── blinky_gcc_nrf52.ld
├── main.c
├── Makefile
├── sdk_config.h
└── include
    ├── arm_acle.h
    ├── arm_cmse.h
    ├── arm_fp16.h
    ├── arm_neon.h
    ├── float.h
    ├── gcov.h
    ├── iso646.h
    ├── mmintrin.h
    ├── stdalign.h
    ├── stdarg.h
    ├── stdatomic.h
    ├── stdbool.h
    ├── stddef.h
    ├── stdfix.h
    ├── stdint-gcc.h
    ├── stdint.h
    ├── stdnoreturn.h
    ├── tgmath.h
    ├── unwind-arm-common.h
    ├── unwind.h
    └── varargs.h

    1 directory, 25 files
```

To do so,

```
$ mkdir ~/example_blinky
$ cd ~/example_blinky
```

Scratch below items from each directory to the example_blinky:
- ~/nRF52/nRF52_SDK/examples/peripheral/**blinky**/main.c
- ~/nRF52/nRF52_SDK/examples/peripheral/**blinky/pca10040/s132/config**/sdk_config.h
- ~/nRF52/nRF52_SDK/examples/peripheral/**blinky/pca10040/s132/armgcc**/Makefile
- ~/nRF52/nRF52_SDK/examples/peripheral/**blinky/pca10040/s132/armgcc**/blinky_gcc_nrf52.ld
- All other files in the pca10040 directory can be ignored

The include files might be located at 
- $HOME/nRF52/gcc-arm/arm-none-eabi/include

<br/><br/>

### 2.4 Edit Makefile for the simplified project

To adjust the **Makefile** for our project, which has the new and simple structure:

* SDK_ROOT := $(HOME)/nRF52/nRF52_SDK
* PROJECT_DIR := .
* For INC_FOLDERS, remove the line of ../config
* For OPT, Optimization level can be 0 (if you want)

Build again:

```
$ cd ~/example_blinky
$ make

...
DONE nrf52832_xxaa
```

Then the _build directory might have some image files newly generated (hex and bin) for debugging.

<br/><br/>

## 3. Setting VSCODE as the IDE 

<br/>

### 3.1 Get VSCODE and some extensions

Download the installer from:
- https://code.visualstudio.com/download

Some extensions are required:
- C/C++
- C++ Intellisense
- Cortex-Debug
- Vim (optional)

Once VSCODE and the extensions are ready:

```
$ cd example_blinky
$ code .
```
 
<br/><br/>

### 3.2 Set Tasks.json for make/clean  
  
To generate a Tasks.json, 
- Press **CTRL+Shift+P**. 
- Then type **task**.
- Lastly click **Tasks: Configure Task**.
  
Above action generates a file (Tasks.json) under .vscode.  
By filling the json file, we can invoke make and make clean with shortcuts.
  
```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "make",
            "command": "make",
            "args": [
                "VERBOSE=1"
            ],
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "problemMatcher": []
        },
        {
            "label": "clean",
            "command": "make",
            "args": [
                "clean"
            ],
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "problemMatcher": []
        }
    ]
}
```
  
With this configuration, we can use **CTRL+SHIFT+B** to open the task dialog and run make or clean.  
  
The detail can be found from MS' document:
- [building-your-c-application-with-visual-studio-code](https://devblogs.microsoft.com/cppblog/building-your-c-application-with-visual-studio-code/)
      
<br/><br/>

### 3.3 Set Launch.json for debugging
  
To generate a launch.json, 
- Press **CTRL+Shift+P**. 
- Then type **launch**.
- Lastly click **Debug: Open launch.json** and **Cortex Debug**.
  
Above action generates a file (launch.json) under .vscode.  
By filling the json file, we can invoke the arm-none-eabi-gdb and JLink GDB server.  
  
```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Cortex Debug",
            "cwd": "${workspaceRoot}",
            "executable": "./_build/nrf52832_xxaa.out",
            "request": "launch",
            "type": "cortex-debug",
            "servertype": "jlink",
            "device": "nrf52",
            "interface": "swd",
            "armToolchainPath": "/usr/bin",
            "gdbpath": "/usr/bin/gdb-multiarch"
        }
    ]
}
```
  
With this configuration, we can use **F5** to start a debugging session.  
  
Two things need to be adjusted based on your project:
- executable: THE_EXECUTABLE_NAME should ba updated.
- device: the depends on the target MCU.
  
The detail can be found from the extension's website ([>>>](https://marcelball.ca/projects/cortex-debug/cortex-debug-launch-configurations/)).  

<br/><br/>

### 3.4 Get Python tools

To test nRF fimware there are well known python tools - nrfutil and pybluez.

Those should be used to update/test the firmware.

```
$ sudo apt install python-pip
$ sudo pip install --ignore-installed six nrfutil
$ sudo pip install pybluez
```

<br/><br/>

## 4. Conclusion

In this walkthrough,

- Downloaded and installed the SDK, tools, and IDE
- Learned how to make your own project by scratching
- Compiled and debugged the project
- Used VSCODE as the IDE

...and everything could be well done in Linux!