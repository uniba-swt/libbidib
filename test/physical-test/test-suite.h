
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

//misc
typedef struct {
    int stateError;
    int stateNotReached;
    int stateNotReachedVerified;
    int stateReached;
    int stateReachedVerified;
    int unknownState;
} Point_result_t;

typedef struct {
    Point_result_t* points;
} Test_result_t;

typedef struct {
  char** ids;
  size_t length;
} sortOutIds_t;

Test_result_t* initTestSuite();


t_bidib_id_list_query sortOutIds(t_bidib_id_list_query idQuery, sortOutIds_t soid );

void logTestResult(Test_result_t *test, t_bidib_unified_accessory_state_query state, int i);
void printTestResults(Test_result_t *test);
void testSignal(t_bidib_id_list_query signals);
void testPointParalel(t_bidib_id_list_query points, Test_result_t* test);
void driveTo(t_bidib_train_position_query *pos, char* segment, int speed, char* train);
void driveToStop(t_bidib_train_position_query *pos, char* segment, int speed, char* train);
void testPointSerial(t_bidib_id_list_query points, Test_result_t* test);

void SWTbahn_std_all_segments(t_bidib_train_position_query *pos, char* train);



