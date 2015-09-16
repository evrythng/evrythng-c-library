# evrythng-c-library

A core C library for the EVRYTHNG API

## EVRYTHNG configuration

First, you need to create a free developer account for the [EVRYTHNG API](https://dashboard.evrythng.com).
Create a new configuration file before compiling: copy `Config_example` to `Config` and edit its content.

## Doxygen documentation

To read API documentation type
```
make docs
```
and open index file with the browser, in Ubuntu
```
gnome-open docs/html/index.html
```

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
