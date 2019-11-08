# physical-test

## Features
Test cases:
1. Switching all the  servos paralell (no delay)
2. Switching all the servos serial (delay)
3. Full track coverage (1 train )
4. 2 Trains paralell
5. Switching all signals

Test cases 1 and 2 prints the libary feedback of the points.

## Dependencies

[swtbahn-cli/configurations/](https://github.com/uniba-swt/swtbahn-cli/tree/master/configurations)


## Build
To build the test-suite use the following commands:
`mkdir bin && cd bin`
`cmake ..`
`make`
## Usage
General Usage:
`./test-suite <testCase> <timesToTest> `

For example:
`./test-suite 1 500`
Switches all the points paralell 500 times.

The order in which points/signals got from libary:
1. `bidib_get_connected_points()`
2. `bidib_get_connected_signals()`

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
