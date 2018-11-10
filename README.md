# libbidib
A library for communication with a BiDiB (www.bidib.org) system using a serial
connection. The library was developed to be compatible with BiDiB revision 1.27.

The library was initially developed by
[Nicolas Gross](https://github.com/nicolasgross) in a student project at the
Software Technologies Research Group (SWT) of the University of Bamberg. Its
purpose is to provide an implementation of the BiDiB protocol as a basis for
controlling a model railway. The railway should later on support teaching of the
upcoming course Reactive Systems Design.


## Features
* Can send all messages which are not marked as local or deprecated (BiDiB
revision 1.27, released on 07.04.2017.)
* Can send and receive messages concurrently (except of `bidib_send_sys_reset()`)
* Creates the allocation table automatically
* Stores the current state of boards, points, signals, peripherals and trains
* Automatically handles many uplink messages by updating the corresponding states
* Adds an abstraction layer which lets you define IDs for boards, points,
signals, peripherals and trains, so that you don't have to mess around with
unique IDs and node addresses (if you want to, you can still use the low level
functions)
* In the log every so called "highlevel" action is assigned to an action id
and every "lowlevel" action which belongs to this highlevel action is also
assigned to the same action id, which makes the effects of the highlevel action
traceable (default value is 0)
* Allows for initial values and active features which are automatically set at
startup
* IDs, initial values and active features are configured via yaml files
* Puts multiple messages in one packet to save bandwidth
* Automatically flushes the buffer for creating packets every x (user defined) ms
* Considers the response capacity of the nodes and stores messages until enough
capacity is free
* If nevertheless a MSG_STALL is received, the library stores all messages to
the affected node and its subnodes until the node signals free capacity
* Uses syslog for logging


## Dependencies
* libglib-2.0, libpthread, and libyaml
  * For macOS, one can use [homebrew](https://brew.sh)
* a [syslog](https://en.wikipedia.org/wiki/Syslog) compatible log daemon
* [cmocka](https://github.com/clibs/cmocka) for the tests
* [cmake](https://cmake.org) is used for installation convenience (depends 
on [pkg-config](https://www.freedesktop.org/wiki/Software/pkg-config/))
* [doxygen](http://www.stack.nl/~dimitri/doxygen/) for building the documentation as HTML
* [lcov](http://ltp.sourceforge.net/coverage/lcov.php) for generating the coverage test report


## Build
1. Clone the repository
2. Adjust `CMakeLists.txt` according to your installations of the dependencies
(libglib-2.0, libpthread, libyaml and cmocka).
3. Navigate to the directory where the build files should be generated
4. Execute `cmake <path-to-project-root>`
5. Choose static or shared library:
	* Shared: Execute `make bidib` which will create the shared library `libbidib.so`
	* Static: Execute `make bidib_static` which will create the static library
	`libbidib_static.a`

For running the tests, execute `make` and afterwards `ctest -V`.

For generating the documentation, execute `doxygen doxygen_config.txt` in the
root directory of the project. This will create the directory `doc` which
contains the documentation.

For generating the code coverage report, run 
`cmake -DCMAKE_BUILD_TYPE=Debug <path-to-project-root>`
and afterwards `make coverage_test`.


## Message handling
Many messages are already handled by the library. The remaining messages are
placed either in the message queue or in the error queue. These queues should be
checked by the user to react appropriately. Here is an overview which messages
will be put in the queues:

#### Error queue
* MSG_SYS_ERROR
* MSG_NODE_NA
* MSG_FEATURE_NA
* MSG_LC_NA
* MSG_ACCESSORY_STATE (only in case of an error)
* MSG_ACCESSORY_NOTIFY (only in case of an error)
* MSG_CS_DRIVE_EVENT (only in case of an error)
* MSG_BOOST_STAT (only in case of an error)

#### Message queue
* MSG_SYS_PONG
* MSG_SYS_P_VERSION
* MSG_SYS_UNIQUE_ID
* MSG_SYS_SW_VERSION
* MSG_SYS_IDENTIFY_STATE
* MSG_VENDOR
* MSG_VENDOR_ACK
* MSG_STRING
* MSG_CS_PROG_STATE
* MSG_ACCESSORY_PARA
* MSG_LC_CONFIGX
* MSG_LC_MACRO_STATE
* MSG_LC_MACRO
* MSG_LC_MACRO_PARA
* MSG_CS_POM_ACK
* MSG_BM_POSITION
* MSG_BM_CV
* MSG_BM_XPOM
* MSG_BM_RCPLUS
* MSG_FW_UPDATE_STAT


## Terminology
For some names/identifiers in the library it's maybe not clear what's meant by
them. Here is a short explanation for the most important ones:

* board:
A BiDiB board (e.g., GBMboost, OneControl, or LightControl).
* point/signal board:
A point/signal that is connected directly to a board.
* point/signal dcc:
A point/signal that is controlled via DCC.
* track output:
A board that has ports to connect to segments and can send DCC commands to them.
* segment:
The rails are split into so called segments that are electrically isolated from each 
other and connect to a track output.
* accessory:
A component that is involved with critical operations (e.g., points and signals).
* peripheral:
A component which does not affect the track operations (e.g., light decorations beside
the rails).
* aspect:
A state of a component (e.g., for a point this could be "normal" or "reverse", 
and for a signal this could be "go" or "stop").
* node address:
The address of a board that is dynamically assigned at system bootup.
* unique id:
The globally unique identifier of a board.

For further information have a look at the [BiDiB specification](http://bidib.org).


## Usage
Use with care, only parts of the functionality was tested on a real system!

The library logs to LOCAL0, you may have to configure your log daemon accordingly.

1. Create the yaml configuration files for your setup, examples can be found
in `project-root/config-example` 
  * You must keep the the elements in the configuration files in the same order as in the example files!
  * points-board -> points-dcc -> signals-board -> signals-dcc -> peripherals -> segments
2. Include bidib.h (`project-root/include/bidib.h`)
3. Start the library, with either `bidib_start_serial(<params>)` or
`bidib_start_pointer(<params>)`
4. Use the library:
	* Read messages: `bidib_read_message()` (Queue capacity: 128 messages)
	* Read error messages: `bidib_read_error_message()` (Queue capacity: 128 messages)
	* Send messages via low level functions: `bidib_send_<message>(<params>)`
	* Send messages via high level function, e.g.:
	`bidib_set_train_speed(char *train, int speed)`
	* Flush the buffer manually: `bidib_flush()`
5. Stop the library: `bidib_stop()`

Calling the functions mentioned in 4. before/while the library is started,
while/after it is stopped or while it is reset (`bidib_send_sys_reset()`) is
not allowed and will result in undefined behaviour.

All the public functions of the library are documented in their respective header files,
found in the `project-root/include` folder.

## Threadsafety
Once the library is started, you can safely call `bidib_read_message()`,
`bidib_read_error_message()`, all low level and high level send functions and
`bidib_flush()` in a concurrent manner. If you stop the library or reset the
system, you have to ensure that none these functions are called during the
execution of `bidib_stop()` and `bidib_send_sys_reset()`.

To summarize:
It's not supported to call **ANY** library function concurrently with
`bidib_start_serial(<params>)`, `bidib_start_pointer(<params>)`, `bidib_stop()`
and `bidib_send_sys_reset()`.


## Licensing
The copyright holder for `bidib_messages.h` is Wolfgang Kufer and the file is
excluded from the GPL3.0 license. The reason behind this is to prevent others
from defining alternative message numbers, which would lead to incompatible
products. The rest of the library is released under the terms of GPL3.0.
