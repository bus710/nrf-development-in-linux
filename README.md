# nRF Development in Linux

## Referenece

* [https://leavesified.wordpress.com/2016/03/24/setup-nrf51-development-on-linux/](https://leavesified.wordpress.com/2016/03/24/setup-nrf51-development-on-linux/)

## Environment

* Mint Linux 18.3 64bit

## Index

* Get Basic Tools
* Get ARM-GCC Compiler
* Get nRF SDK
* Get JLink Package
* Get nrfjprog and Flash SoftDevice
* Modify Makefile.Posix for SDK
* Compile an Example
* Edit Makefile for the scratched project
* Get Eclipse and MCU/vrapper Packages
* Import the Example to Eclipse
* Set Parser Configuration
* Set Debug Perspective with nrfjprog Script

### Get Basic Tools

```
$ sudo apt update
$ sudo apt install build-essential
$ sudo apt install eclipse
$ sudo apt install openocd
$ sudo apt install git
```

### Get ARM-GCC Compiler

```
$ sudo add-apt-repository ppa:team-gcc-arm-embedded/ppa
$ sudo apt update
$ sudo apt install gcc-arm-embedded
$ arm-none-eabi-gcc -v
```

## Get nRF SDK

Download the compressed file from [https://developer.nordicsemi.com/nRF51\_SDK/](https://developer.nordicsemi.com/nRF51_SDK/)

I downloaded:

* nRF5\_SDK\_14.2.0\_17b948a.zip
* nRF5\_SDK\_14.2.0\_offline\_doc.zip

Uncompress the files.

```
$ mkdir ~/nRF5 ~/Desktop/offline_doc
$ unzip nRF5_SDK_14.2.0_17b948a.zip -d ~/nRF5
$ unzip nRF5_SDK_14.2.0_offline_doc.zip -d ~/Desktop/offline_doc
```

### Get JLink Package

Download **J-Link Software and Documentation pack for Linux, DEB Installer, 64-bit **from [https://www.segger.com/downloads/jlink/\#J-LinkSoftwareAndDocumentationPack](https://www.segger.com/downloads/jlink/#J-LinkSoftwareAndDocumentationPack)

Currently the version is 6.30a

```
$ sudo dpkg -i JLink_Linux_V630_x86_64.deb
```

To test it, connect your eval board or JLink to your PC and...

```
$ JLinkExe -device nrf52 -speed 1000 -if swd

// Then will see

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

## Get nrfjprog and Flash Softdevice

Download **nRF5x-Command-Line-Tools\_9\_7\_2\_Linux-x86\_64.tar** from [https://www.nordicsemi.com/eng/nordic/Products/nRF52840/nRF5x-Command-Line-Tools-Linux64/58852](https://www.nordicsemi.com/eng/nordic/Products/nRF52840/nRF5x-Command-Line-Tools-Linux64/58852)

```
$ mkdir !/nRF5Tools
$ cd nRF5Tools
$ cp ~/Downloads/nRF5x-Command-Line-Tools_9_7_2_Linux-x86_64.tar ~/nRF5Tools
$ tar xvf nRF5x-Command-Line-Tools_9_7_2_Linux-x86_64.tar

// Need to add nrfjprog directory path to PATH.
// For example, I added:

// export PATH=$PATH:~/nRF5Tools/nrgjprog
// export PATH=$PATH:~/nRF5Tools/mergehex
```

To rewrite a softdevice,

```
$ cd ~/nRF5/components/softdevice/s132/hex
$ nrfjprog -f NRF52 --program s132_nrf52_5.0.0_softdevice.hex --chiperase
```

## Modify Makefile.Posix for SDK

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

## Compile an Example

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
    |   `-- 이하 생략
    |-- main.c
    `-- sdk_config.h

    1 directory, 23 files

To do so,

```
$ cd SOMEWHERE
$ mkdir example_blinky

// Scratch files from ~/nRF5/examples/peripheral/blinky/pca10040/s132/armgcc
// But the include might be located at /usr/lib/gcc/arm-none-eabi/7.2.1/include
```

## Edit Makefile for the scratched project

To adjust the Makefile for our project, which has the new and simple structure:

* SDK\_ROOT := $\(HOME\)/nRF5
* PROJECT\_DIR := .
* For INC\_FOLDERS, remove a dot of ../config
* For OPT, Optimization level can be 0 \(if you want\)

And try compiling:

```
$ cd THE_PROJECT_DIRECTORY
$ make
```

Then the \_build directory might have some new image files for debug.

## Get Eclipse and MCU/vrapper Packages

To install minimum dev environment:

* Get Eclipse Neon 3 \(Not Oxygen or Luna\).
* Extract it and run Eclipse\_Inst
* Once it is done, go help &gt; Market Place.
* Search and install MCU and vrapper

To set the tool chain and j-link's path:

* Run Eclipse
* Go to Window &gt; Preferences &gt; MCU &gt; Global ARM Toolchains Paths
* Set the path to /usr/bin
* Go to Window &gt; Preferences &gt; MCU &gt; Global SERGGER J-Link Paths
* Set the path to /opt/SEGGER/JLink

## Import the Example to Eclipse

To import the example project we made:

* File &gt; New &gt; Project 
* In the Wizard's first screen, C/C++ &gt; Makefile project with existing file
* In the Wizard's second screen, browse the project location and select ARM GCC.

## Set Parser Configuration



## Set Debug Perspective with nrfjprog Script



