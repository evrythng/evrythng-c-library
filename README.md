# evrythng-c-library
A core C library for the EVRYTHNG API

## Requirements

In order to compile the library and run tests you should:

1. have the following software installed:

* Standart GCC toolchain
* make
* CMake
* libssl-dev

The library was build and tested using the following versions of the software:

* Ubuntu x86_64 with 3.16.0-33-generic kernel
* gcc (Ubuntu 4.8.2-19ubuntu1) 4.8.2
* GNU Make 3.81
* cmake version 2.8.12.2
* libssl-dev version 1.0.1f-1ubuntu9

2. Make a copy of `Config_example` as `Config` and edit it by
filling in the correct values corresponding to your [EVRYTHNG account](https://dashboard.evrythng.com/) content.
You should create a [Product](https://dashboard.evrythng.com/documentation/api/products), [Thng](https://dashboard.evrythng.com/documentation/api/thngs) and generate a [Device Application Key](https://dashboard.evrythng.com/documentation/api/thngs#thngs-devices).

## Building the lib

Building the library, demo application and tests is as easy as typing:
```
make
```
the debug version is built as a default in the `build_debug` directory. 

To build a release version dp:
```
make DEBUG=0
```
the release version will be built in the `build` directory.

Additionally you can set `VERBOSE` to 1 to see the full `cmake` output (0 by default) 
and `BUILD_DIR` option to change the output build directory. 

For example the command:
```
make DEBUG=0 BUILD_DIR=release VERBOSE=1
```
will build the release version using `release` directory with the full `cmake` output.

To clean the output directory type:
```
make clean
```
To delete the output directory:
```
make cleanall
```
or simply `rm -rf build_dir`

Note: that you should use the `DEBUG` and `BUILD_DIR` options with all `make` commands if you used them to build the library.

## Documentation

To generate the Doxygen API documentation type:
```
make docs
```
and open the index file with your browser (`docs/html/index.html`)

## Demo application

After sucessfull compilation you can launch the demo application via the `${build_dir}/evrythng-demo` command:
```
./build_debug/evrythng-demo -h
```
this will print the demo app help. 

### Options

* To establish a secure connection to `ssl://mqtt.evrythng.com` add the: `-c ./misc/client.pem` option.

* Use the `--pub` option to let the demo applicaton update the property with random values ([0,100]) every 2 seconds.

Additionaly you can use `./demo.sh` to run the demo application.

## Tests

To run the tests type:

```
make runtests
```
