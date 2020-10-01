#include <stdio.h>

#include <stdio.h>

#include <unistd.h>

#include <bidib.h>

#include <stdlib.h>

#include <string.h>

#define WAITINGTIME 3

t_bidib_id_list_query points;
t_bidib_id_list_query signals;
t_bidib_unified_accessory_state_query state;
t_bidib_train_position_query pos;

//testsuite structures
typedef struct {
        int stateError;
        int stateNotReached;
        int stateNotReachedVerified;
        int stateReached;
        int stateReachedVerified;
        int unknownState;
}
t_testsuite_point_result;

typedef struct {
        Point_result_t * points;
}
t_testsuite_test_result;

typedef struct {
        char ** ids;
        size_t length;
}
t_testsuite_sortOutIds;

//setup
t_testsuite_test_result* testsuite_initTestSuite();
t_bidib_id_list_query testsuite_sortOutIds(t_bidib_id_list_query idQuery, t_testsuite_sortOutIds soid);

//logging
void testsuite_logTestResult(t_test_result * test, t_bidib_unified_accessory_state_query state, int i);
void testsuite_printTestResults(t_testsuite_test_result * test);

//driving
void testsuite_checkTrain(char * train);
void testsuite_driveTo(t_bidib_train_position_query * pos, char * segment, int speed, char * train);
void testsuite_driveToStop(t_bidib_train_position_query * pos, char * segment, int speed, char * train);

//Test cases
void testCase_Signal(t_bidib_id_list_query signals);
void testCase_PointParalel(t_bidib_id_list_query points, t_testsuite_test_result * test);
void testCase_PointSerial(t_bidib_id_list_query points, t_testsuite_test_result * test);
void testCase_SWTbahn_std_all_segments(t_bidib_train_position_query * pos, char * train);
