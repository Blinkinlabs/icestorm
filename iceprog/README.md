## Build

To build Iceprog simply run

```
$ make
```

If a prefix for the compiled firmware is wanted use for example

```
$ make PROGRAM_PREFIX=tillitis-
```
to produce a program named `tillitis-iceprog`

Use

```
sudo make PROGRAM_PREFIX=tillitis- install
```
to install the program in `/usr/local/bin`


### macOS

To build on macOS use

```
$ make OS=macos
```

Pre-requisites for building on macOS is the library [`libusb`](https://github.com/libusb/libusb).

Build and install it by downlaoding the source and read the instructions in `INSTALL` in `libusb`.

The short version is: download and unpack, enter the folder and run

```
$ ./configure
$ make
$ make install
```