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
* Modify Makefile.Posix for SDK
* Flash SoftDevice
* Compile an Example
* Get Eclipse and MCU/vrapper Packages
* Import the Example to Eclipse
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

Download the compressed file from https://developer.nordicsemi.com/nRF51\_SDK/

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

Download **J-Link Software and Documentation pack for Linux, DEB Installer, 64-bit **from https://www.segger.com/downloads/jlink/\#J-LinkSoftwareAndDocumentationPack 

Currently the version is 6.30a

```
$ sudo dpkg -i JLink_Linux_V630_x86_64.deb
```

To test it, connect your eval board or JLink to your PC and...

```
$ JLinkExe -device nrf52832 -speed 1000 -if swd
```

To erase SoftDevice...

```
$ cd ~/nRF5/components/softdevice/s132/hex/
$ JLinkExe -device nrf52832 -speed 1000 -if swd
```



