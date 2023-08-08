# Physical Test Suite

Tests the main functionalities of the BiDiB library on a real SWTbahn platform, 
while also testing the platform's mechanical reliability.

```
physical
  |-- swtbahn-standard
  |     |-- main.c
  |     |-- testsuite.c
  |     '-- testsuite.h
  |-- swtbahn-full
  |     |-- main.c
  |     |-- testsuite.c
  |     '-- testsuite.h
  |-- Readme.md
  |-- test_common.c
  '-- test_common.h
```

## Test Cases

1. **Switch all the point servos simultaneously (parallel):**   
   Commands all the point servos to switch to the same position at the same time. 
   This simulates the worst-case situation that the OneControl BiDiB boards need 
   to deliver maximum power. The test case waits 3 seconds between each command.
	
2. **Switch all the point servos one after the other (serial):**
    Commands each point servo to switch to the same position, one after another a time.
	The test case waits 3 seconds between each command.

3. **Switch all signals in parallel:**
    Commands all signals to cycle through all their aspects at the same time. 
	The test case waits 3 seconds between each command.

4. **Track coverage using one train:**
    > **WARNING**: This test case can only be executed on specific SWTbahn platforms!
	
    Drives a user-defined train along the track segments. 
    
    For the SWTbahn Standard, the train must be facing anti-clockwise on track segment `T1`.
    For the SWTbahn Full, the train must be facing clockwise on track segment `T58`.
	
5. **Track coverage using two trains:**
    > **WARNING**: This test case can only be executed on specific SWTbahn platforms!
	
	Drives two user-defined trains along the track segments. 
	
    For the SWTbahn Full, the first train must be facing clockwise on track segment `T58`, 
    and the second train must be facing clockwise on track segment `T46`.


For a given SWTbahn platform, its points and signals are retrieved using the 
libbidib functions `bidib_get_connected_points()` and `bidib_get_connected_signals()`.
These functions may also return the accessories that do not need to be tested, e.g., 
platform lights, lantern power outputs, or synchronisation pulses. These accessories 
can be ignored with a guard, such as,
```
if (!strcmp(signals.ids[i], "platformlights")) { continue; }
```
The function `testsuite_filterOutIds()` uses this approach to filter out
a given array of accessory names to ignore.

The order in which the points and signals are returned is decided by their 
textual order in the `bidib_track_config.yml` configuration file.
For each point and signal, the `bidib_switch_point()` and `bidib_set_signal()`
functions are used to set their aspect.

The `t_testsuite_point_result` struct records the feedback state returned by a point, and is
updated by the `testsuite_logTestResult()` function. The following is an example of the 
feedback that is logged for a point:

```
point10
  -> stateReachedVerified: 314
  -> stateReached: 2
  -> stateNotReachedVerified: 0
  -> stateNotReached: 0
  -> stateError: 284
  -> unknownState: 0
```


## Dependencies

**SWTbahn Platform**
*  Physical access to an SWTbahn platform
*  For the SWTbahn platform that has been chosen for testing, its configuration files need
   to be available in the path `../../swtbahn-cli/configurations/swtbahn-???` relative to the test executable. 
   You can find platform specific configuration folders in 
   [/swtbahn-cli/configurations/](https://github.com/uniba-swt/swtbahn-cli/tree/master/configurations)
* Points, signals and trains must be defined in your configuration file  


## Usage

Use the following command to run a particular test case by specifying its 
`number`, `repetition`, and `trainName`s if needed:

```
./testsuite <testCaseNumber> <repetition> [trainName] [trainName]
```

For example, `./test-suite 1 500` switches all the points, 500 times in parallel, and 
`./test-suite 4 5 cargo_bayern` drives the train cargo_bayern along the tracks, 5 times.
See your configuration file for possible train names.

The test case results are displayed in the terminal, but can be redirected
to a file for archiving or later viewing:

```
./testsuite | tee outputFile.txt
./testsuite > outputFile.txt
```

A test case execution can be terminated by typing
**Ctrl-C** into the terminal. This sends a `SIGINT` signal to the program, which
executes a callback function to free the heap memory, to set the train
speeds to zero, and to terminate libbidib.


## Limitations

The test suite is a small selection of cases for catching obvious hardware problems and to stress test common functionality.

* There may be platform specific commands that are executed by the test cases. For example, the track coverage test case is 
  tailored to a specific track layout (SWTbahn Standard and Full).
* The test suite can only give a rough indication of the platform's mechanical reliability, and cannot give a detailed
  hardware verification results.
* No feedback is given for the signal test case or the track coverage test case.
* The track coverage test case does not drive the train along all tracks in both directions.
