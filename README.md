# nRF Development in Linux

If you need/want to have nRF development environment in Linux...

<br/><br/>

---

## Environment

* Ubuntu Linux 19.10 Eoan 64bit \(or equivalent distro\)
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
   - Edit Makefile for the scratched project

3. Setting VSCODE as the IDE 
   * Get Eclipse and MCU/vrapper Packages
   * Import the Example to Eclipse
   * Set Build Option and Parser Configuration
   * Set Debug Configuration in Eclipse
   * Set Debug Launcher in C/C++ Perspective
   * Get Python tools

4. Conclusion

<br/><br/>

---

## 1. Getting the SDK and tools

### 1.1 Get Basic Tools

```
$ sudo apt update
$ sudo apt install build-essential git libncurses5
```

If libncurses5 installation fails:

```
$ sudo apt --fix-broken install
```

<br/><br/>

### 1.2 Get ARM-GCC Compiler

For those who want the latest release of the compiler:

```
$ sudo add-apt-repository ppa:team-gcc-arm-embedded/ppa
$ sudo apt update
$ sudo apt install gcc-arm-embedded
$ arm-none-eabi-gcc -v
```

However, I installed the stable version since Eoan is not being supported yet:

```
$ sudo apt update
$ sudo apt install gcc-arm-none-eabi
$ arm-none-eabi-gcc -v
```

<br/><br/>

### 1.3 Get nRF SDK

Download the compressed file from [https://developer.nordicsemi.com/nRF51_SDK/nRF5_SDK_v16.x.x/](https://developer.nordicsemi.com/nRF51_SDK/nRF5_SDK_v16.x.x/)

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

### 2.3 Simplifying the example

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

### 2.4 Edit Makefile for the scratched project

To adjust the **Makefile** for our project, which has the new and simple structure:

* SDK_ROOT := $(HOME)/nRF52_SDK
* PROJECT_DIR := .
* For INC_FOLDERS, remove the line for ../config
* For OPT, Optimization level can be 0 (if you want)

And try compiling:

```
$ cd ~/example_blinky
$ make

...
DONE nrf52832_xxaa
```

Then the _build directory might have some new image files (hex and bin) for debugging.


<br/><br/>

---

## 3. Setting VSCODE as the IDE 

WIP

### Get Eclipse and MCU/vrapper Packages

To install minimum dev environment:

* Get Eclipse Neon 3 \(Not Oxygen or Luna\).
* Extract it and run Eclipse\_Inst
* Once it is done, go help &gt; Market Place.
* Search and install MCU and vrapper

To set the tool chain and j-link's path:

* Run Eclipse
* Go to Window &gt; Preferences &gt; MCU &gt; Global ARM Toolchains Paths
* Set the path to **/usr/bin**
* Go to Window &gt; Preferences &gt; MCU &gt; Global SERGGER J-Link Paths
* Set the executable name to **JLinkGDBServerCLExe**
* Set the path to **/opt/SEGGER/JLink**

<br/><br/>

### Import the Example to Eclipse

To import the example project we made:

* File &gt; New &gt; Project 
* In the Wizard's first screen, C/C++ &gt; Makefile project with existing file
* In the Wizard's second screen, browse the project location and select ARM GCC.

<br/><br/>

### Set Build Option and Parser Configuration

Build Option should be changed:

* Go to Project &gt; Properties &gt; C/C++ Build &gt; Builder Settings &gt; Build Command
* Edit it as the image \(make VERBOSE=1\)

![](/assets/20180201a.png)

Parser Option should be changed:

* Go to Project &gt; Properties &gt; C/C++ General &gt; Preprocessor Include Pathes, Macros etc.
* Go to Providers tap &gt; Click CDT GCC Build Output Parser Item
* Change Compiler command pattern

![](/assets/20180201b.png)

![](/assets/20180201c.png)

Compiler Option should be changed:

* Go to Project &gt; Properties &gt; C/C++ General &gt; Preprocessor Include Pathes, Macros etc.
* Go to Providers tap &gt; Click CDT ARM Cross GCC Build-in Compiler Settings
* Change Command to get compiler specs

![](/assets/20180201d.png)

![](/assets/20180201e.png)

Everything is done!

Go to Project &gt; Clean. That will clean and build the project.

<br/><br/>

### Set Debug Configuration in Eclipse

Finally it is time to try debugging.

* Go to Run &gt; Debug Configurations
* Right click **GDB SEGGER J-Link Debugging** and **New**
* Click the second tap "Debugger"
* Actual executable for the server should be **"JLinkGDBServerCLExe"**
* Device name should be **"nrf52"**
* Executable for the client should be **"arm-none-eabi-gdb"**
* Click Apply and Debug

![](/assets/20180201f.png)

<br/><br/>

### Set Debug Launcher in C/C++ Perspective

Sometimes C/C++ Perspective doesn't show Debug Launch button on its tap.

* Go to Window &gt; Perspective &gt; Customize Perspective &gt; Tool Bar Visibility
* Activate Debug Item in the list.

<br/><br/>

### Get Python tools

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

- Downloaded the compiler, IDE, and SDK
- Learned how to make your own project by scratching
- Compiled and debugged the project
- Used VSCODE as the IDE

...and everything could be well done in Linux!