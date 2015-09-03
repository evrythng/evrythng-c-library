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

## Programming guide

Writing a client for EVRYTHNG cloud using the с library can be splitted into the following steps:
* Declare, initialize and setup a handle of type `evrythng_handle_t` used to make all library api calls
* Connect to the EVRYTHNG cloud
* Subscribe or publish properties/actions/locations using appropriate api calls
* Disconnect and deinitialize handle when you are done

### Initializing and setting up a handle

All library api calls require a handle as a first parameter to operate. Basically this handle is an opaque pointer to a structure which contains private and user provided data to operate with the cloud. The structure is not exposed to the user of the library and known only to the core functionality, so do not try to access members of it directly and use api calls to set it up. The following code snippet demonstrates the minimum (hence mandatory) sequence of calls to initialize a handle:
```
evrythng_handle_t handle;

EvrythngInitHandle(&handle);
EvrythngSetUrl(handle, "tcp://mqtt.evrythng.com:1883");
EvrythngSetKey(handle, "<your api key here>");
```
By analyzing the url provided the library will decide whether to establish a secured connection or not. If your url starts with **tcp://** an unsecured connection will be established and if it starts with **ssl://** - secured. Note that the port at the end of url is mandatory. So to establish an unsecured connection you'll use an url like **tcp://mqtt.evrythng.com:1883**, and for secured **ssl://mqtt.evrythng.com:443**. All other settings will have default values. Here is a snippet to setup optional settings:
```
EvrythngSetLogCallback(handle, log_callback); /* default: null pointer */
EvrythngSetConnectionCallbacks(handle, on_connection_lost, on_connection_restored); /* default: null pointers */
EvrythngSetClientId(handle, "<client id>); /* default: a 10 bytes string of random numbers */
EvrythngSetQos(handle, 1); /* 0,1 or 2, default: 1*/
EvrythngSetThreadPriority(handle, 1); /* any meaningfull priority for the underlying OS, default: 0 */
EvrythngSetThreadStacksize(handle, 4096); /* default: 8192 */
```
The meaning of some settings (regarding thread and callbacks) will be become clear in the next section.

### Connecting to the EVRYTHNG cloud

The api call to connect to the cloud is `EvrythngConnnect`. Normally you would like to do something like this in the very beginning of you app:
```
while (EvrythngConnect(evt_handle) != EVRYTHNG_SUCCESS)
{
  log("Retrying");
  sleep(3);
}
```
Internally the library launches a thread for managing all communication with the cloud. Priority and stack size of it can be configured using api calls listed above. The library automatically reconnects to the cloud and restores subcriptions in case of connection was lost. Your application can be notified about the fact that connection was lost and restored by providing callbacks via api call `EvrythngSetConnectionCallbacks`. These callbacks are only for doing some stuff specifiс to your application. Callbacks are called in the context of internal library thread. Please, do not try to connect/disconnect or use any other api calls inside these callbacks as it will lead to internal thread lock.

### Working with the cloud

After a connection is successfully established you can start using api calls subscribe to and publish properties/actions/locations using appropriate api calls. It is possible to publish/subscribe from different threads of your application as the library is thread safe.

### Finalizing

When you are done working with the cloud you should disconnect and deninitilaize the handle to avoid any resource leaks:
```
EvrythngDisconnect(handle);
EvrythngDestroyHandle(handle);
```
