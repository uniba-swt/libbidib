# physical-test

The test-suite is small software for testing the physical functionalities of our SWTbahn platforms. It's designed to work with the libbidib only.

## Features

Test cases:
1. Switching all the  servos paralell (no delay)
    The case simulates the startup switching process. It's designed to stresstest the platform in the roughest possible form.
2. Switching all the servos serial (delay)
    The case switches the points with a small delay you can define.
3. Full track coverage (1 train )
    The case lets a defined train drive through all the segments available.
    *NOTE*: This test is platform dependent! You can only use it with the *SWTbahn standard*!
4. 2 Trains paralell
    The case lets two defined train drive in two defined circles.
    Also the test case is good for small presentations of the platform.
    *NOTE*: This test is platform dependent! You can only use it with the *SWTbahn standard*!
5. Switching all signals
    The case switches the signals all at once in a defined color pattern.



The test cases 1, 2, 5 are using the `bidib_get_connected_points()` and the `bidib_get_connected_signals()` for `getting a t_bidib_id_list_query`.
The software simply using the query to use with `bidib_switch_point(points.ids[i], "Accessory");` or `bidib_set_signal(signals.ids[i], "Accessory");`
 to set the Accessorystate for the point/signal.

 You can also use the Point_result struct for getting a rough overview of the onset of the different states the libbidib supports.
 It is filled by function `logTestResult(test, state, i);` which uses the  `t_bidib_unified_accessory_state_query` and `bidib_get_point_state(points.ids[i]);` for getting the accesory states.

This is a sample output of test case 1, which is close to the output the test cases 2,3 are generating.
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

In order to use the libbidib you will need to have configeration files which are describing your platform.
You cann see in [examples/configs](../../example/config) how to create the config files.
Or you can download the configeration of a specific platform:
[swtbahn-cli/configurations/](https://github.com/uniba-swt/swtbahn-cli/tree/master/configurations)


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
  |---bin/
  |     |---test-suite
  |     |---Makefile
  |
  |---configerations/
  |     |---configerations directory (eg. SWTbahn-standard/)
  |
  |---CmakeLists.txt
  |---main.c
  |---Readme.md

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
