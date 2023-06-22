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

To install the program in `/usr/local/bin` use
```
sudo make PROGRAM_PREFIX=tillitis- install
```

### macOS
To build on macOS use

```
$ make OS=macos
```

Pre-requisites for building iceprog is the library
[`libusb`](https://github.com/libusb/libusb). For most Linux distros
this comes pre-installed, but for macOS it needs to be installed
manually. Simplest is through Homebrew using

```
$ brew install libusb
```
