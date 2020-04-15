#include <stdio.h>
#include <unistd.h>
#include <bidib.h>
#include "test-suite.h"
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#define WAITINGTIME 4
extern


extern t_bidib_id_list_query points;
extern t_bidib_id_list_query signals;
extern t_bidib_unified_accessory_state_query state;
extern t_bidib_train_position_query pos;

void logTestResult(Test_result *test, t_bidib_unified_accessory_state_query state, int i);


void signal_callback_handler(int signum){
	bidib_free_id_list_query(points);
	bidib_free_id_list_query(signals);
    bidib_free_unified_accessory_state_query(state);
    bidib_free_train_position_query(pos);
    bidib_set_train_speed("cargo_bayern", 0, "master");
    bidib_stop();
    printf("SIGINT - stopping libbidib \n");
    exit(signum);

	}


int main(int argc, char** args) {
	int rounds = 0;
	printf("Test-Suite\n");
	signal(SIGINT, signal_callback_handler);
    if(argc < 3){
		printf("Usage: ./test-suite TestCaseNum Rounds\n Cases:\n 1- Points paralell\n 2- Points serial \n 3- Track coverage \n 5- Signals paralell \n");
		return 0;
		}
	else if((argc > 3) && !(atoi(args[1]) == 6)){
		printf("Usage: ./test-suite TestCaseNum Rounds\n Cases:\n 1- Points paralell\n 2- Points serial \n 3- Track coverage \n 5- Signals paralell \n");
		return 0;
		}
	else if((argc > 3) && (atoi(args[1]) == 6)){
		printf(" \n");

		}
	else if( !((atoi(args[1]) == 1) || (atoi(args[1]) == 2) || (atoi(args[1]) == 5) || (atoi(args[1]) == 3) || (atoi(args[1]) == 4) || (atoi(args[1]) == 6))){
		printf("Usage: ./test-suite TestCaseNum Rounds\n Cases:\n 1- Points paralell\n 2- Points serial \n 3- Track coverage \n 5- Signals paralell \n");
		return 0;
		}


    if(bidib_start_serial("/dev/ttyUSB0", "../../../../swtbahn-cli/configurations/swtbahn-standard", 200)){

        printf("failed to start\n");
        return 0;
    }

	points = bidib_get_connected_points();
	signals = bidib_get_connected_signals();
	int t;
    Test_result *test = malloc(sizeof(Test_result));
    test->points = malloc(points.length * sizeof(Point_result));

    for(int i = 0; i < points.length; i++){
        test->points[i].stateReachedVerified = 0;
        test->points[i].stateReached = 0;
        test->points[i].stateNotReachedVerified = 0;
        test->points[i].stateNotReached = 0;
        test->points[i].stateError = 0;
        test->points[i].unknownState = 0;
    }

	if(atoi(args[1]) == 1) {
        rounds = atoi(args[2]);
        t = 0;
        while (t < rounds) {
            printf("Test case 1\n");
            testPointSerial(points, test);
            t++;
        }
    }
	else if(atoi(args[1]) == 2) {
        rounds = atoi(args[2]);
        t = 0;
        while (t < rounds) {
            printf("Test case 1\n");
            testPointParalel(points, test);
            t++;
        }
    }
	else if(atoi(args[1]) == 3){

      }

	else if(atoi(args[1]) == 4){

      }

      bidib_free_train_position_query(pos);
	  bidib_free_id_list_query(signals);
    bidib_free_id_list_query(points);
    bidib_free_unified_accessory_state_query(state);
    free(test);

    bidib_stop();

    return 0;
}

void logTestResult(Test_result *test, t_bidib_unified_accessory_state_query state, int i){
		if (state.known) {
                switch (state.board_accessory_state.execution_state) {
                    case BIDIB_EXEX_STATE_ERROR:
                        test->points[i].stateError++;
                        break;
                    case BIDIB_EXEC_STATE_NOTREACHED:
                        test->points[i].stateNotReached++;
                        break;
                    case BIDIB_EXEC_STATE_NOTREACHED_VERIFIED:
                        test->points[i].stateNotReachedVerified++;
                        break;
                    case BIDIB_EXEC_STATE_REACHED:
                        test->points[i].stateReached++;
                        break;
                    case BIDIB_EXEC_STATE_REACHED_VERIFIED:
                        test->points[i].stateReachedVerified++;
                        break;
                    default:;
                        break;
                }

            } else {
                test->points[i].unknownState++;
            }
	}
