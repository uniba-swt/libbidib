# physical

Tests the main functionalities of the BiDiB library on a real SWTbahn platform, 
while also testing the mechanical reliability of the platform.

```
physical
  |-- build
  |   |-- testsuite
  |   '-- Generated makefile
  |
  |-- configurations
  |   '-- SWTbahn platform configuration directory (eg. swtbahn-standard)
  |
  |-- main.c
  |-- testsuite.c
  |-- testsuit.h
  |-- CmakeLists.txt
  '-- Readme.md
```

## Test Cases

1. **Switch all the point servos simultaneously (parallel):**
   Commands all the point servos to switch at the same position at the same time. 
   This simulates the worst-case situation that the OneControl BiDiB boards need 
   to deliver maximum power. The test case waits between every state change 3 seconds.
	
2. **Switch all the point servos one after the other (serial):**
    Commands each point servo to switch to the same position one at a time, with a 
	configurable delay between each command. The test case waits between every state and accessory change 3 seconds.

3. **Track coverage using one train:**
    > **WARNING**: This test can only be executed on the **SWTbahn Standard**!
	
    Drives the `cargo_bayern` train through all the track segments. Train must be 
    facing anticlockwise on track segment `T1`.
	
4. **Empty test case
	
5. **Switch all signals:**
    Cycles through all aspects of all signals at the same time. The test case waits between every state change 3 seconds.

For a given SWTbahn platform, its points and signals are retrieved using the 
`bidib_get_connected_points()` and `bidib_get_connected_signals()` functions.
These functions will also return accessories that do not need to be tested, e.g., 
lantern power outputs or synchronisation pulse. These accessories can ignored 
in the code with the following guard:
```
if(strcmp(points.ids[i], "name of accessory to ignore")) { ... }
```

The order in which the points and signals are returned is decided by their 
textual order in the `bidib_track_config.yml` configuration file.
For each point and signal, the `bidib_switch_point(...)` and `bidib_set_signal(...)`
functions are used to set their aspect.

The `Point_result` struct records the feedback state returned by a point, and is
updated by the `logTestResult(...)` function. The following is an example of the 
feedback that is logged for a point:

```
Point: point10
unknown state: 0
state reached: 2
state unreached: 0
state reched verified: 314
state unreached verified: 0
state error: 284
```


## Dependencies

**SWTbahn Platform**
*  Physical access to an SWTbahn platform
*  For the SWTbahn platform that has been chosen to execute the tests, its configuration folder
   needs to be placed into [`configurations`](configurations). You can find platform specific 
   configuration folders in [swtbahn-cli/configurations/](https://github.com/uniba-swt/swtbahn-cli/tree/master/configurations)
* Points, signals and trains must be configured in your configuration file  


## Usage


Use the following command to run a particular test case by specifying its 
`number` and `repetition`:

```
./testsuite <test case number> <repetition>
```

To use different trains, you need to name the `train-id` (see configurations files) as third argument.

```
./testsuite <test case number> <repetition> <train-id>
```

Before running test cases 3 the train need to be placed on the following track segments:
* Test case 3: `<train-id>` anticlockwise on track segment `T1`


For example, `./test-suite 1 500` switches all the point servos together, 500 times.
			`./test-suite 3 5 cargo_bayern` drives the train with the train-id: cargo_bayern 5 times over all segments.

The test case results are displayed in the terminal, but can be redirected
to a file for archiving or later viewing:

```
./testsuite | tee outputFile.txt
./testsuite > outputFile.txt
```

A test case execution can be terminated by entering
**Ctrl-C** in the terminal. This sends a `SIGINT` signal to the program, which
executes a callback function to free the heap memory, to set the train
speeds to zero, and to terminate libbidib.


## Limitations

The test-suite is only a selection of small test cases we used to verify our hardware automaticly and to stress test our systems.

Please note:

* You can't run the test-suite on your modelrailway without modifying the code
* The test-suite is not suited for hardware-verification in the scientific sense but more an indication of basic hardware functionality
* You can't get software feedback from the test case `signal` (5) or `swtbahnStandardTrackCoverage` (3)
	
	
