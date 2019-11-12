# nRF Development in Linux

If you need/want to have nRF development environment in Linux...

<br/><br/>

---

## Environment

* Ubuntu Linux 18.04 (or later) 64bit
* nRF52832 DK board

<br/><br/>

---

## Index

1. Getting the SDK and tools
   - Basic Tools
   - Get ARM-GCC Compiler
   - Get nRF SDK
   - Get nrfjprog and Flash SoftDevice
   - Get JLink Package (Optional)

2. Building the example
   - Modify Makefile.Posix for SDK
   - Compile an Example
   - Simplify the example
   - Edit Makefile for the simplified project

3. Setting VSCODE as the IDE 
   - Get VSCODE and some extensions
   - Set Tasks.json for make/clean  
   - Set Launch.json for debugging
   - Get Python tools

4. Conclusion

<br/><br/>

---

## 1. Getting the SDK and tools

### 1.1 Get Basic Tools

```
$ sudo apt update
$ sudo apt install build-essential \
                    git \
                    libncurses5 \
                    gdb-multiarch
```

If libncurses5 installation fails:

```
$ sudo apt --fix-broken install
```

<br/><br/>

### 1.2 Get ARM-GCC Compiler

To install the stable version:

```
$ sudo apt update
$ sudo apt install gcc-arm-none-eabi
$ arm-none-eabi-gcc -v
```

To install the latest version (This may not support Eoan):

```
$ sudo add-apt-repository ppa:team-gcc-arm-embedded/ppa
$ sudo apt update
$ sudo apt install gcc-arm-embedded
$ arm-none-eabi-gcc -v
```

<br/><br/>

### 1.3 Get nRF SDK

Download the compressed file from:
-  [https://developer.nordicsemi.com/nRF51_SDK/nRF5_SDK_v16.x.x/](https://developer.nordicsemi.com/nRF51_SDK/nRF5_SDK_v16.x.x/)

I downloaded **nRF5_SDK_16.0.0_98a08e2.zip** and uncompressed the file:

```
$ mkdir ~/nRF52_SDK
$ unzip nRF5_SDK_16.0.0_98a08e2.zip -d ~/nRF52_SDK
```

<br/><br/>

### 1.4 Get nrfjprog and Flash Softdevice

Download **nRF5x-Command-Line-Tools** for Linux 64 bit from:
- [https://www.nordicsemi.com](https://www.nordicsemi.com/Software-and-tools/Development-Tools/nRF-Command-Line-Tools/Download#infotabs)

Run these commands to install the tools:

```
$ tar xvf nRF-Command-Line-Tools_10_4_1_Linux-amd64.tar.gz
$ sudo dpkg -i JLink_Linux_V650b_x86_64.deb
$ sudo dpkg -i nRF-Command-Line-Tools_10_4_1_Linux-amd64.deb
```

To check the version of installed tools:

```
$ nrfjprog -v

nrfjprog version: 10.4.1 
JLinkARM.dll version: 6.50b

$ mergehex -v

mergehex version: 10.4.1

$ JLinkExe -v

SEGGER J-Link Commander V6.50b (Compiled Sep  6 2019 17:46:52)
DLL version V6.50b, compiled Sep  6 2019 17:46:40

Unknown command line option -h. (<= Don't worry about this)
```

To flash a softdevice to a connected nRF board via JLink,

```
$ cd ~/nRF52_SDK/components/softdevice/s132/hex
$ ls

s132_nrf52_7.0.1_licence-agreement.txt  
s132_nrf52_7.0.1_softdevice.hex

$ nrfjprog -f NRF52 --program s132_nrf52_7.0.1_softdevice.hex --chiperase
```

<br/><br/>

### 1.5 Get JLink Package (Optional)

If Nordic's command line tool doesn't include the JLink package, follow below steps.

Download **J-Link Software and Documentation pack for Linux, DEB Installer, 64-bit** from [https://www.segger.com/downloads/jlink/\#J-LinkSoftwareAndDocumentationPack](https://www.segger.com/downloads/jlink/#J-LinkSoftwareAndDocumentationPack)

Currently the version is 6.54c:

```
$ sudo dpkg -i JLink_Linux_V654c_x86_64.deb
```

To test it, connect your eval board or JLink to your PC and...

```
$ JLinkExe -device nrf52 -speed 1000 -if swd
```

Then will see:

```
SEGGER J-Link Commander V6.30a (Compiled Jan 31 2018 18:14:21)
DLL version V6.30a, compiled Jan 31 2018 18:14:14

Connecting to J-Link via USB...Updating firmware:  J-Link OB-SAM3U128-V2-NordicSemi compiled Jan 12 2018 16:05:20
Replacing firmware: J-Link OB-SAM3U128-V2-NordicSemi compiled Jul 24 2017 17:30:12
Waiting for new firmware to boot
New firmware booted successfully
O.K.
Firmware: J-Link OB-SAM3U128-V2-NordicSemi compiled Jan 12 2018 16:05:20
Hardware version: V1.00
S/N: XXXXXXXXX
VTref = 3.300V

> connect
> exit
```

<br/><br/>

---

## 2. Building the example
  
### 2.1 Modify Makefile.Posix for SDK

Get the information of the installed compiler:

```
$ which arm-none-eabi-gcc

/usr/bin/arm-none-eabi-gcc

$ arm-none-eabi-gcc -v

...
gcc version 7.3.1 20180622 (release) [ARM/embedded-7-branch revision 261907] (15:7-2018-q2-6) 
```

Edit Makefile:

```
$ cd ~/nRF52_SDK/components/toolchain/gcc
$ vi Makefile.posix
```

Then make the file looks like based on the information we got:

```
GNU_INSTALL_ROOT ?= /usr/bin/
GNU_VERSION ?= 7.2.1
GNU_PREFIX ?= arm-none-eabi
```

<br/><br/>

### 2.2 Compile an Example

Now we can just compile an example.

```
$ cd ~/nRF52_SDK/examples/peripheral/blinky/pca10040/s132/armgcc
$ ls 

blinky_gcc_nrf52.ld 
Makefile

$ make

...
DONE nrf52832_xxaa
```

<br/><br/>

### 2.3 Simplify the example

However, it is highly recommended to scratch to make a slim project structure like this:

```
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
- ~/nRF52_SDK/examples/peripheral/**blinky**/main.c
- ~/nRF52_SDK/examples/peripheral/**blinky/pca10040/s132/config**/sdk_config.h
- ~/nRF52_SDK/examples/peripheral/**blinky/pca10040/s132/armgcc**/Makefile
- ~/nRF52_SDK/examples/peripheral/**blinky/pca10040/s132/armgcc**/blinky_gcc_nrf52.ld
- All other files in the pca10040 directory can be ignored

The include might be located at 
- /usr/lib/gcc/arm-none-eabi/7.2.1/include

<br/><br/>

### 2.4 Edit Makefile for the simplified project

To adjust the **Makefile** for our project, which has the new and simple structure:

* SDK_ROOT := $(HOME)/nRF52_SDK
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

---

## 3. Setting VSCODE as the IDE 

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
- [building-your-c-application-with-visual-studio-code](https://blogs.msdn.microsoft.com/vcblog/2016/10/24/building-your-c-application-with-visual-studio-code/).
      
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

---

## 4. Conclusion

In this walkthrough,

- Downloaded and installed the SDK, tools, and IDE
- Learned how to make your own project by scratching
- Compiled and debugged the project
- Used VSCODE as the IDE

...and everything could be well done in Linux!