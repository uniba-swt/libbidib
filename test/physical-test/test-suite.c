#include "test-suite.h"


t_testsuite_test_result * testsuite_initTestSuite() {
        char ** exAc = malloc(sizeof(char * ) * 4);
        exAc[0] = "sync1";
        exAc[1] = "sync2";
        exAc[2] = "lanterns1";
        exAc[3] = "lanterns2";

        t_testsuite_sortOutIds soID;
        soID.length = 4;
        soID.ids = exAc;

        points = testsuite_sortOutIds(bidib_get_connected_points(), soID);

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
                printf("\n\nPoint%d \n", i);
                printf("  -> stateReachedVerified: %d \n", test -> points[i].stateReachedVerified);
                printf("  -> stateReached: %d \n", test -> points[i].stateReached);
                printf("  -> stateNotReachedVerified: %d \n", test -> points[i].stateNotReachedVerified);
                printf("  -> stateNotReached: %d \n", test -> points[i].stateNotReached);
                printf("  -> stateError: %d \n", test -> points[i].stateError);
                printf("  -> unknownState: %d \n", test -> points[i].unknownState);
        }
}

t_bidib_id_list_query testsuite_sortOutIds(t_bidib_id_list_query idQuery, t_testsuite_sortOutIds soid) {

        size_t count;
        count = idQuery.length - soid.length;

        if (count <= 0) {
                printf("No ids left after sorting out\n");
        }

        t_bidib_id_list_query iq;
        iq.length = count;
        iq.ids = malloc(sizeof(char * ) * count);

        int k = 0, isSoId = 0;

        for (int i = 0; i < idQuery.length; i++) {
                isSoId = 0;
                for (int j = 0; j < soid.length; j++) {
                        if (!strcmp(idQuery.ids[i], soid.ids[j])) {
                                free(idQuery.ids[i]);
                                isSoId = 1;
                        }
                }
                if (!isSoId) {
                        iq.ids[k] = idQuery.ids[i];
                        k++;
                }
        }

        if (k != count) {
                printf("error ids mismatch %d %d\n", count, k);
        }
        return iq;
}

void testsuite_checkTrain(char * train) {
        if (bidib_get_train_on_track(train)) {
                if (strcmp("seg1", bidib_get_train_position(train).segments[0])) {
                        printf("%s is not on  segment 1\n", train);
                }
                printf("%s selected\n", train);

        } else {
                printf("%s is not on track\n", train);
        }
}

void testsuite_driveTo(t_bidib_train_position_query * pos, char * segment, int speed, char * train) {
        bidib_set_train_speed(train, speed, "master");
        while (1) {
                * pos = bidib_get_train_position(train);
                sleep(1);
                for (int i = 0; i < pos -> length; i++) {
                        if (!strcmp(pos -> segments[i], segment)) {
                                return;
                        }
                }
        }
}

void testsuite_driveToStop(t_bidib_train_position_query * pos, char * segment, int speed, char * train) {
        bidib_set_train_speed(train, speed, "master");
        while (1) {
                * pos = bidib_get_train_position(train);
                sleep(1);
                for (int i = 0; i < pos -> length; i++) {
                        if (!strcmp(pos -> segments[i], segment)) {
                                bidib_set_train_speed(train, 0, "master");
                                return;
                        }
                }
        }
}

void testCase_Signal(t_bidib_id_list_query signals) {
        for (int i = 0; i < points.length; i++) {
                //printf("testing %s  \n", points.ids[i]);
                bidib_set_signal(signals.ids[i], "on");
                state = bidib_get_point_state(points.ids[i]);
        }

        for (int i = 0; i < points.length; i++) {
                //printf("testing %s  \n", points.ids[i]);
                bidib_switch_point(points.ids[i], "normal");
                state = bidib_get_point_state(points.ids[i]);
        }
}

void testCase_PointParalel(t_bidib_id_list_query points, t_testsuite_test_result * test) {
        for (int i = 0; i < points.length; i++) {
                //printf("testing %s  \n", points.ids[i]);
                bidib_switch_point(points.ids[i], "reverse");
                state = bidib_get_point_state(points.ids[i]);
                testsuite_logTestResult(test, state, i);
        }

        for (int i = 0; i < points.length; i++) {
                //printf("testing %s  \n", points.ids[i]);
                bidib_switch_point(points.ids[i], "normal");
                state = bidib_get_point_state(points.ids[i]);
                testsuite_logTestResult(test, state, i);
        }

}

void testCase_PointSerial(t_bidib_id_list_query points, t_testsuite_test_result * test) {
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

        sleep(WAITINGTIME);
}

void testCase_SWTbahn_std_all_segments(t_bidib_train_position_query * pos, char * train) {

        checkTrain(train);

        bidib_switch_point("point1", "normal");
        driveTo(pos, "seg4", 20, train);
        bidib_switch_point("point2", "normal");
        bidib_switch_point("point3", "normal");
        bidib_switch_point("point4", "normal");
        driveTo(pos, "seg12", 50, train);
        bidib_switch_point("point6", "reverse");
        bidib_switch_point("point8", "reverse");
        bidib_switch_point("point2", "reverse");
        bidib_switch_point("point3", "reverse");
        bidib_switch_point("point4", "reverse");
        driveTo(pos, "seg23", 30, train);
        bidib_switch_point("point5", "reverse");
        bidib_switch_point("point9", "reverse");
        bidib_switch_point("point10", "reverse");
        bidib_switch_point("point11", "reverse");
        bidib_switch_point("point12", "normal");
        driveToStop(pos, "seg37", 30, train);
        bidib_switch_point("point12", "reverse");
        driveToStop(pos, "seg40", -30, train);
        bidib_switch_point("point12", "normal");
        bidib_switch_point("point9", "normal");
        bidib_switch_point("point10", "normal");
        bidib_switch_point("point11", "normal");
        driveToStop(pos, "seg28", 30, train);
        bidib_switch_point("point3", "normal");
        bidib_switch_point("point4", "normal");
        driveTo(pos, "seg21", 30, train);
        bidib_switch_point("point5", "normal");
        bidib_switch_point("point7", "normal");
        bidib_switch_point("point6", "normal");
        driveToStop(pos, "seg28", 30, train);
        bidib_switch_point("point7", "reverse");
        bidib_switch_point("point6", "normal");
        bidib_switch_point("point8", "normal");
        bidib_switch_point("point1", "reverse");
        driveToStop(pos, "seg4", 50, train);
        bidib_switch_point("point1", "normal");
        bidib_switch_point("point7", "normal");
        driveToStop(pos, "seg1", -20, train);
}
