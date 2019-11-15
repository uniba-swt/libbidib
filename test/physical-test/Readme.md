# physical-test

Tests the main functionalities of the BiDiB library on a real SWTbahn platform, 
what at the same time tests the mechanical reliability of the platform.

```
physical-test
  |-- bin
  |   |-- test-suite
  |   '-- Makefile
  |
  |-- configurations
  |   '-- SWTbahn platform configuration directory (eg. swtbahn-standard)
  |
  |-- main.c
  |-- CmakeLists.txt
  '-- Readme.md
```

## Test Cases

1. **Switch all the point servos simultaneously (parallel):**
   Commands all the point servos to switch at the same position at the same time. 
   This simulates the worst-case situation that the OneControl BiDiB boards need 
   to deliver maximum power.
	
2. **Switch all the point servos one after the other (serial):**
    Commands each point servo to switch to the same position one at a time, with a 
	configurable delay between each command.

3. **Track coverage using one train:**
    > **WARNING**: This test can only be executed on the **SWTbahn Standard**!
    Drives the `cargo_bayern` train through all the track segments. Train must be 
    facing anticlockwise on track segment `T1`.
	
4. **Two trains on the track together:**
    > **WARNING**: This test can only be executed on the **SWTbahn Standard**!
    Drives the `cargo_bayern` and `regional_odeg` trains together, one on each 
	independent loop. Can be used to showcase the SWTbahn to an audience.
	The `cargo_bayern` train must facing anticlockwise on track segment `T2`
	and the `regional_odeg` train must be facing anticlockwise on track
	segment `T37`.
	
5. **Switch all signals:**
    Cycles through all aspects of all signals at the same time.

For a given SWTbahn platform, its points and signals are retrieved using the 
`bidib_get_connected_points()` and `bidib_get_connected_signals()` functions.
The order in which the points and signals are returned is decided by their 
textual order in the `bidib_track_config.yml` configuration file.
For each point and signal, the `bidib_switch_point(...)` and `bidib_set_signal(...)`
functions are used to set their aspect.

The `Point_result` struct records the feedback state returned by a point, and is
updated by the `logTestResult(...)` function. Example of the overall feedback 
that is logged for a point:
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
*  Trains `cargo_bayern` and `regional_odeg`
*  For the SWTbahn platform that has been chosen to execute the tests, its configuration folder
   needs to be placed into [`configurations`](configurations). You can find platform specific 
   configuration folders in [swtbahn-cli/configurations/](https://github.com/uniba-swt/swtbahn-cli/tree/master/configurations)


## Build

1. Navigate to the directory where the test suite should be generated
2. Execute `cmake <path-to-physical-test-root>`
3. Execute `make`   


## Usage

Before running test cases 3 and 4, trains need to be placed on the following track segments:
* Test case 3   Place the train (cargo_bayern) on segment 1
* Test case 4   Place fist train (cargo_bayern) on segment 2 and the second train on segment 37

Use the following command to run a particular test case by specifying its 
`number` and how many times `repeat` it:

```
./test-suite <test case number> <repeat>
```

The test case results that are displayed in the terminal can be redirected
to a file for archiving or later viewing:

```
./test-suite | tee outputFile.txt
./test-suite > outputFile.txt
```


**Terminating a test case**
Enter **Ctrl-C** in the terminal to send a `SIGINT` signal to the program. 
A callback function is executed to free heap allocated memory, set the train
speeds to zero, and to terminate libbidib.


For changing the trains you will need to change it in the code.

For example:
`./test-suite 1 500`
Switches all the points paralell 500 times.

NOTE it will also give you the accessories which you might not ant to use.
Eg. points also includes the sync accessory.
You can simply remove them by using the following piece of code:
```
if(!strcmp(points.ids[i], "unwanted accessory")){

    }
```
