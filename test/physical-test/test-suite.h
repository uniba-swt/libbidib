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
    t_testsuite_point_result * points;
}
t_testsuite_test_result;

typedef struct {
    char ** ids;
    size_t length;
}
t_testsuite_ids;

//setup
t_testsuite_test_result * testsuite_initTestSuite();
t_bidib_id_list_query testsuite_filterOutIds(t_bidib_id_list_query inputIdQuery, t_testsuite_ids filterOutIds);

//logging
void testsuite_logTestResult(t_testsuite_test_result * test, t_bidib_unified_accessory_state_query state, int i);
void testsuite_printTestResults(t_testsuite_test_result * test);

//driving
int testsuite_checkTrain(char * train);
void testsuite_driveTo(char * segment, int speed, char * train);
void testsuite_driveToStop(char * segment, int speed, char * train);
 
//Test cases
void testsuite_case_signal();
void testsuite_case_pointParallel(t_testsuite_test_result * test);
void testsuite_case_pointSerial(t_testsuite_test_result * test);
void testsuite_case_swtbahnStandardTrackCoverage(char * train);
