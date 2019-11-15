# physical-test

Tests the main functionalities of the BiDiB library on a real SWTbahn platform, 
what at the same time tests the mechanical reliability of the platform.


## Test Cases
1. **Switch all the point servos simultaneously (parallel):**
   Commands all the point servos to switch at the same position at the same time. 
   This simulates the worst-case situation that the OneControl BiDiB boards need 
   to deliver maximum power.
	
2. **Switch all the point servos one after the other (serial):**
    Commands each point servo to switch to the same position one at a time, with a 
	configurable delay between each command.

3. **Track coverage using one train:**
    Drives a specific train through all the track segments. 
    > **WARNING**: This test can only be executed on the **SWTbahn Standard**!

4. **Two trains on the track together:**
    Drives two specific trains together, one on each independent loop. Can be used to showcase
	the SWTbahn to an audience.
    > **WARNING**: This test can only be executed on the **SWTbahn Standard**!
	
5. **Switch all signals:**
    Cycles through all aspects of all signals at the same time.

For a given SWTbahn platform, its points and signals are retrieved using the 
`bidib_get_connected_points()` and `bidib_get_connected_signals()` functions.
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
*  For the SWTbahn platform that has been chosen to execute the tests, its configuration folder
   needs to be placed into [`configurations`](configurations). You can find platform specific 
   configuration folders in [swtbahn-cli/configurations/](https://github.com/uniba-swt/swtbahn-cli/tree/master/configurations)


## Build

Navigate to `libbidib/test/physical-test/`
Use `mkdir bin && cd bin ` to create and navigate to the `bin` directory
Create a Makefile by using `cmake ..`
and compile with `make`   
You should get a executable which is name test-suite.

Below you see the directory in a tree structure.
After the build you should get this structure and files.
NOTE there will be much more files which are not displayed below for the sake of simplicity.

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


## Usage

General Usage:

`./test-suite <testCase> <timesToTest> `

You can also use pipes:

`./test-suite | tee outputFile.txt` 

`./test-suite > outputFile.txt` 


Stopping a test case:
You can stop a test case by using `STRG - C`.
The software uses the SIGINT signal. The callback function is just cleaning up. eg. stopping libbidib  

Physical preperations for using the test cases:
- Test case 1   none
- Test case 2   none
- Test case 3   Place the train (cargo_bayern) on segment 1
- Test case 4   Place fist train (cargo_bayern) on segment 2 and the second train on segment 37
- Test case 5   none

For changing the trains you will need to change it in the code.

For example:
`./test-suite 1 500`
Switches all the points paralell 500 times.

As mentioned the test-suite uses the `t_bidib_id_list_query` with the functions `bidib_get_connected_points()` and `bidib_get_connected_signals()`
This is importand to understand in which order the software tests the equipment.
For the SWTbahn-standard the functions gives you the id query in the following order:

* Points:
  * OneControl (OC1)
    * point2
    * point3
    * point4
    * point5
    * point7
    * point8
  * OneControl (OC2)
    * point1
    * point6
    * point9
    * point10
    * point11
    * point12

* Signals:
  * LightControl (LC1)
    * signal3
    * signal4
    * signal5
    * signal6
    * signal7
    * signal11
    * signal12
    * signal13
    * signal17
    * signal19
  * (LC2)
    * signal1
    * signal2
    * signal8
    * signal9
    * signal10
    * signal14
    * signal15
    * signal16
    * signal18

NOTE it will also give you the accessories which you might not ant to use.
Eg. points also includes the sync accessory.
You can simply remove them by using the following piece of code:
```
if(!strcmp(points.ids[i], "unwanted accessory")){

    }
```
