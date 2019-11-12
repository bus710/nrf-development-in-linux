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

* Get Basic Tools
* Get ARM-GCC Compiler
* Get nRF SDK
* Get nrfjprog and Flash SoftDevice
* Get JLink Package
* Modify Makefile.Posix for SDK
* Compile an Example
* Edit Makefile for the scratched project
* Get Eclipse and MCU/vrapper Packages
* Import the Example to Eclipse
* Set Build Option and Parser Configuration
* Set Debug Configuration in Eclipse
* Set Debug Launcher in C/C++ Perspective
* Get Python tools
* Conclusion

<br/><br/>

---

## 1. Get Basic Tools

```
$ sudo apt update
$ sudo apt install build-essential git libncurses5
```

If libncurses5 installation fails:

```
$ sudo apt --fix-broken install
```

<br/><br/>

## 2. Get ARM-GCC Compiler

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

## 3. Get nRF SDK

Download the compressed file from [https://developer.nordicsemi.com/nRF51_SDK/nRF5_SDK_v16.x.x/](https://developer.nordicsemi.com/nRF51_SDK/nRF5_SDK_v16.x.x/)

I downloaded **nRF5_SDK_16.0.0_98a08e2.zip** and uncompressed the file:

```
$ mkdir ~/nRF5
$ unzip nRF5_SDK_16.0.0_98a08e2.zip -d ~/nRF5
```

<br/><br/>

## 4. Get nrfjprog and Flash Softdevice

Download **nRF5x-Command-Line-Tools** for Linux 64 bit from:
- [https://www.nordicsemi.com/Software-and-tools/Development-Tools/nRF-Command-Line-Tools/Download#infotabs](https://www.nordicsemi.com/Software-and-tools/Development-Tools/nRF-Command-Line-Tools/Download#infotabs) 

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

Unknown command line option -h.
```

To rewrite a softdevice,

```
$ cd ~/nRF5/components/softdevice/s132/hex
$ ls

s132_nrf52_7.0.1_licence-agreement.txt  
s132_nrf52_7.0.1_softdevice.hex

$ nrfjprog -f NRF52 --program s132_nrf52_7.0.1_softdevice.hex --chiperase
```

<br/><br/>

## 5. Get JLink Package (Optional)

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

## 6. Modify Makefile.Posix for SDK

Getting Information

```
$ which arm-none-eabi-gcc

>> /usr/bin/arm-none-eabi-gcc

$ arm-none-eabi-gcc -v

>> gcc version 7.2.1 20170904
```

Editing Makefile

```
$ cd ~/nRF5/components/toolchain/gcc
$ vi Makefile.posix

// Then make the file looks like based on the information we got:

GNU_INSTALL_ROOT := /usr/bin/
GNU_VERSION := 7.2.1
GNU_PREFIX := arm-none-eabi
```

<br/><br/>

## 7. Compile an Example

Now we can just compile an example.

```
$ cd ~/nRF5/examples/peripheral/blinky/pca10040/s132/armgcc
$ make
```

However, it is highly recommended to scratch to make a slim project structure like this:

    $ tree
    .
    |-- Makefile
    |-- blinky_gcc_nrf52.ld
    |-- include
    |   |-- arm_acle.h
    |   `-- (and more...)
    |-- main.c
    `-- sdk_config.h

    1 directory, 23 files

To do so,

```
$ cd SOMEWHERE
$ mkdir example_blinky

// Scratch files from
//   ~/nRF5/examples/peripheral/blinky/
//   ~/nRF5/examples/peripheral/blinky/pca10040/s132/armgcc

// The include might be located at /usr/lib/gcc/arm-none-eabi/7.2.1/include

// Also the files in 
//   ~/nRF5/examples/peripheral/blinky/pca10040/blank/armgcc
// are not really required since those need to be used when SoftDevice is not written.
```

<br/><br/>

## 8. Edit Makefile for the scratched project

To adjust the Makefile for our project, which has the new and simple structure:

* SDK\_ROOT := $\(HOME\)/nRF5
* PROJECT\_DIR := .
* For INC\_FOLDERS, remove the line for ../config
* For OPT, Optimization level can be 0 \(if you want\)

And try compiling:

```
$ cd THE_PROJECT_DIRECTORY
$ make
```

Then the \_build directory might have some new image files for debug.


<br/><br/>

## 9. Get Eclipse and MCU/vrapper Packages

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

## 10. Import the Example to Eclipse

To import the example project we made:

* File &gt; New &gt; Project 
* In the Wizard's first screen, C/C++ &gt; Makefile project with existing file
* In the Wizard's second screen, browse the project location and select ARM GCC.

<br/><br/>

## 11. Set Build Option and Parser Configuration

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

## 12. Set Debug Configuration in Eclipse

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

## 13. Set Debug Launcher in C/C++ Perspective

Sometimes C/C++ Perspective doesn't show Debug Launch button on its tap.

* Go to Window &gt; Perspective &gt; Customize Perspective &gt; Tool Bar Visibility
* Activate Debug Item in the list.

<br/><br/>

## 14. Get Python tools

To test nRF fimware there are well known python tools - nrfutil and pybluez.

Those should be used to update/test the firmware.

```
$ sudo apt install python-pip
$ sudo pip install --ignore-installed six nrfutil
$ sudo pip install pybluez
```

<br/><br/>

---

## Conclusion

So far we followed this short manual to set up nRF development environment in Linux.

In the manual,

* Downloaded the compiler, IDE, and SDK
* Learned how to make your own project by scratching
* Compiled and debugged the project.

As software changes everyday, the setup can be changed as well. However the basic step might be not very different.

If you are a BLE-MCU engineer, this nRF setup, KiCAD, and Draft Sight might be helpful to build up a system to develop IoT edge devices.

