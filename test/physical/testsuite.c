#include "testsuite.h"


t_testsuite_test_result * testsuite_initTestSuite() {

	//This is SWTbahn standard specific!
	//If you have pseado accessories leave the array blank
    char* excludedAccessories[4] = {"sync1", "sync2", "lanterns1", "lanterns2"};

    t_testsuite_ids soID;
    soID.length = 4;
    soID.ids = excludedAccessories;

    points = testsuite_filterOutIds(bidib_get_connected_points(), soID);
    signals = bidib_get_connected_signals();
    
    t_testsuite_test_result * test = malloc(sizeof(t_testsuite_test_result));
    test -> points = malloc(points.length * sizeof(t_testsuite_point_result));

    for (int i = 0; i < points.length; i++) {
        test -> points[i].stateReachedVerified = 0;
        test -> points[i].stateReached = 0;
        test -> points[i].stateNotReachedVerified = 0;
        test -> points[i].stateNotReached = 0;
        test -> points[i].stateError = 0;
        test -> points[i].unknownState = 0;
    }
    return test;
}

void testsuite_logTestResult(t_testsuite_test_result * test, t_bidib_unified_accessory_state_query state, int i) {
    if (state.known) {
        switch (state.board_accessory_state.execution_state) {
        case BIDIB_EXEX_STATE_ERROR:
            test -> points[i].stateError++;
            break;
        case BIDIB_EXEC_STATE_NOTREACHED:
            test -> points[i].stateNotReached++;
            break;
        case BIDIB_EXEC_STATE_NOTREACHED_VERIFIED:
            test -> points[i].stateNotReachedVerified++;
            break;
        case BIDIB_EXEC_STATE_REACHED:
            test -> points[i].stateReached++;
            break;
        case BIDIB_EXEC_STATE_REACHED_VERIFIED:
            test -> points[i].stateReachedVerified++;
            break;
        default:
            ;
            break;
        }
    } else {
        test -> points[i].unknownState++;
    }
}

void testsuite_printTestResults(t_testsuite_test_result * test) {

    for (int i = 0; i < points.length; i++) {
        printf("\n\n%s\n", points.ids[i]);
        printf("  -> stateReachedVerified: %d \n", test -> points[i].stateReachedVerified);
        printf("  -> stateReached: %d \n", test -> points[i].stateReached);
        printf("  -> stateNotReachedVerified: %d \n", test -> points[i].stateNotReachedVerified);
        printf("  -> stateNotReached: %d \n", test -> points[i].stateNotReached);
        printf("  -> stateError: %d \n", test -> points[i].stateError);
        printf("  -> unknownState: %d \n", test -> points[i].unknownState);
    }
}

t_bidib_id_list_query testsuite_filterOutIds(t_bidib_id_list_query inputIdQuery, t_testsuite_ids filterOutIds) {

    size_t count;
    count = inputIdQuery.length - filterOutIds.length;

    if (count <= 0) {
        printf("No ids left after sorting out\n");
    }

    t_bidib_id_list_query outputIdQuery;
    outputIdQuery.length = 0;
    outputIdQuery.ids = malloc(sizeof(char * ) * count);

    int isSoId = 0;

    for (int i = 0; i < inputIdQuery.length; i++) {
        isSoId = 0;
        for (int j = 0; j < filterOutIds.length; j++) {
            if (!strcmp(inputIdQuery.ids[i], filterOutIds.ids[j])) {
                free(inputIdQuery.ids[i]);
                isSoId = 1;
            }
        }
        if (!isSoId) {
            outputIdQuery.ids[outputIdQuery.length++] = inputIdQuery.ids[i];
        }
    }

    if (outputIdQuery.length != count) {
        printf("error ids mismatch %d %d\n", count, outputIdQuery.length);
    }
    return outputIdQuery;
}

int testsuite_checkTrain(char * train) {
    if (bidib_get_train_on_track(train)) {
        if (strcmp("seg1", bidib_get_train_position(train).segments[0])) {
            printf("%s is not on  segment 1\n", train);
            return 1;
        }
        printf("%s selected\n", train);
        return 0;

    } else {
        printf("%s is not on track\n", train);
        return 1;
    }
}

void testsuite_driveTo(char * segment, int speed, char * train) {
    bidib_set_train_speed(train, speed, "master");
    while (1) {
        pos = bidib_get_train_position(train);
        sleep(1);
        for (int i = 0; i < pos.length; i++) {
            if (!strcmp(pos.segments[i], segment)) {
                return;
            }
        }
    }
}

void testsuite_driveToStop(char * segment, int speed, char * train) {
    testsuite_driveTo(segment, speed, train);
    bidib_set_train_speed(train, 0, "master");
}

void testsuite_case_signal() {
    for (int i = 0; i < signals.length; i++) {
        //printf("testing %s  \n", points.ids[i]);
        if(strcmp(signals.ids[i],"platformlights")){
        bidib_set_signal(signals.ids[i], "yellow");
		}
	}
	sleep(WAITINGTIME);
	
	for (int i = 0; i < signals.length; i++) {
        //printf("testing %s  \n", points.ids[i]);
        if(strcmp(signals.ids[i],"platformlights")){
        bidib_set_signal(signals.ids[i], "green");
		}
	}
	sleep(WAITINGTIME);
	
	for (int i = 0; i < signals.length; i++) {
        //printf("testing %s  \n", points.ids[i]);
        if(strcmp(signals.ids[i],"platformlights")){
        bidib_set_signal(signals.ids[i], "red");
		}
	}
	sleep(WAITINGTIME);
    
}

void testsuite_case_pointParallel(t_testsuite_test_result * test) {
    for (int i = 0; i < points.length; i++) {
        //printf("testing %s  \n", points.ids[i]);
        bidib_switch_point(points.ids[i], "reverse");
        state = bidib_get_point_state(points.ids[i]);
        testsuite_logTestResult(test, state, i);
    }

    sleep(WAITINGTIME);

    for (int i = 0; i < points.length; i++) {
        //printf("testing %s  \n", points.ids[i]);
        bidib_switch_point(points.ids[i], "normal");
        state = bidib_get_point_state(points.ids[i]);
        testsuite_logTestResult(test, state, i);
    }

}

void testsuite_case_pointSerial(t_testsuite_test_result * test) {
    for (int i = 0; i < points.length; i++) {
        //printf("testing %s  \n", points.ids[i]);
        bidib_switch_point(points.ids[i], "reverse");
        state = bidib_get_point_state(points.ids[i]);
        testsuite_logTestResult(test, state, i);
        sleep(WAITINGTIME);

        bidib_switch_point(points.ids[i], "normal");
        state = bidib_get_point_state(points.ids[i]);
        testsuite_logTestResult(test, state, i);
        sleep(WAITINGTIME);
    }

}

void testsuite_case_swtbahnStandardTrackCoverage(char * train) {

    if(testsuite_checkTrain(train)){
		return;
		}

    bidib_switch_point("point1", "normal");
    testsuite_driveTo("seg4", 20, train);
    bidib_switch_point("point2", "normal");
    bidib_switch_point("point3", "normal");
    bidib_switch_point("point4", "normal");
    testsuite_driveTo("seg12", 50, train);
    bidib_switch_point("point6", "reverse");
    bidib_switch_point("point8", "reverse");
    bidib_switch_point("point2", "reverse");
    bidib_switch_point("point3", "reverse");
    bidib_switch_point("point4", "reverse");
    testsuite_driveTo("seg23", 30, train);
    bidib_switch_point("point5", "reverse");
    bidib_switch_point("point9", "reverse");
    bidib_switch_point("point10", "reverse");
    bidib_switch_point("point11", "reverse");
    bidib_switch_point("point12", "normal");
    testsuite_driveToStop("seg37", 30, train);
    bidib_switch_point("point12", "reverse");
    testsuite_driveToStop("seg40", -30, train);
    bidib_switch_point("point12", "normal");
    bidib_switch_point("point9", "normal");
    bidib_switch_point("point10", "normal");
    bidib_switch_point("point11", "normal");
    testsuite_driveToStop("seg28", 30, train);
    bidib_switch_point("point3", "normal");
    bidib_switch_point("point4", "normal");
    testsuite_driveTo("seg21", 30, train);
    bidib_switch_point("point5", "normal");
    bidib_switch_point("point7", "normal");
    bidib_switch_point("point6", "normal");
    testsuite_driveToStop("seg28", 30, train);
    bidib_switch_point("point7", "reverse");
    bidib_switch_point("point6", "normal");
    bidib_switch_point("point8", "normal");
    bidib_switch_point("point1", "reverse");
    testsuite_driveToStop("seg4", 50, train);
    bidib_switch_point("point1", "normal");
    bidib_switch_point("point7", "normal");
    testsuite_driveToStop("seg1", -20, train);
}
