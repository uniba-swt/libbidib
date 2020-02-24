#include <stdio.h>
#include <stdio.h>
#include <unistd.h>
#include <bidib.h>
#include <stdlib.h>
#include <string.h>


//misc
typedef struct {
    int stateError;
    int stateNotReached;
    int stateNotReachedVerified;
    int stateReached;
    int stateReachedVerified;
    int unknownState;

} Point_result;

typedef struct {
    Point_result* trackEquipment;
    t_bidib_id_list_query points;
    t_bidib_id_list_query signals;

} Test_result;

typedef struct {
  char** ids;
  size_t length;
} sortOutIds;

t_bidib_unified_accessory_state_query state;
t_bidib_train_position_query pos;

t_bidib_id_list_query sortOutIds(t_bidib_id_list_query idQuery, sortOutIds soid );

void logTestResult(Test_result *test, t_bidib_unified_accessory_state_query state, int i);
void testSignal(t_bidib_id_list_query signals);
void testPoint(t_bidib_id_list_query points, Test_result* test);
void driveTo(t_bidib_train_position_query *pos, char** segment);
