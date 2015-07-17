# evrythng-c-library
A core C library for the EVRYTHNG API

## Requirements

In order to compile library and run tests you should:

1. have the following software installed:

* Standart GCC toolchain
* make
* CMake
* libssl-dev

The library was build and tested using the following versions of software:

* Ubuntu x86_64 with 3.16.0-33-generic kernel
* gcc (Ubuntu 4.8.2-19ubuntu1) 4.8.2
* GNU Make 3.81
* cmake version 2.8.12.2
* libssl-dev version 1.0.1f-1ubuntu9

2. Copy Config_example file to Config and edit it by
filling in the correct values according to your Evrythng account content.
You should create product, thing and generate a device application key.

## Building

Building a library, demo application and tests is as easy as typing
```
make
```
by default a debug version is built in "build_debug" directory. 
In order to build a release version you have to type
```
make DEBUG=0
```
release version will be built in "build" directory.

Additionally you can set VERBOSE to 1 to see the full cmake output (0 by default) 
and BUILD_DIR option to change the output build directory. For example the command:
```
make DEBUG=0 BUILD_DIR=release VERBOSE=1
```
will build release version using "release" directory with full cmake output.

To clean the output directory type
```
make clean
```
To delete the output directory
```
make cleanall
```
or simply rm -rf build_dir

Note: that you should use DEBUG and BUILD_DIR options with all make commands if you used it to build the library.

## Doxygen documentation

To read API documentation type
```
make docs
```
and open index file with the browser, in Ubuntu
```
gnome-open docs/html/index.html
```

## Running tests
To run tests type 
```
make runtests
```

## Demo Application

After sucessfull compilation you can launch demo application via ${build_dir}/demo/evrythng-cli command.
```
./build_debug/demo/evrythng-cli -h
```
will print help. Add "-c ./docs/client.pem" option while establishing secure connection to ssl://mqtt.evrythng.com.
Using "--pub" demo applicaton will send random values from [0,100] range to provided property every 2 seconds.

Additionaly you can use a helpfull script ./demo.sh to run demo application.
