#include "test-suite.h"

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


	void testSignal(t_bidib_id_list_query signals){
        for (int i = 0; i < points.length; i++) {

            printf("testing %s  \n", points.ids[i]);
            bidib_set_signal(signals.ids[i], "on");

            state = bidib_get_point_state(points.ids[i]);
            logTestResult(test, state, i);

        }

        for (i = 0; i < points.length; i++) {

            printf("testing %s  \n", points.ids[i]);
            bidib_switch_point(points.ids[i], "normal");

            state = bidib_get_point_state(points.ids[i]);
            logTestResult(test, state, i);

        }
	}

void testPointParalel(t_bidib_id_list_query points, Test_result* test){
    for (int i = 0; i < points.length; i++) {

        printf("testing %s  \n", points.ids[i]);
        bidib_switch_point(points.ids[i], "reverse");

        state = bidib_get_point_state(points.ids[i]);
        logTestResult(test, state, i);

    }

    for (int i = 0; i < points.length; i++) {

        printf("testing %s  \n", points.ids[i]);
        bidib_switch_point(points.ids[i], "normal");

        state = bidib_get_point_state(points.ids[i]);
        logTestResult(test, state, i);

    }

}


void testPointSerial(t_bidib_id_list_query points, Test_result* test){
		for (int i = 0; i < points.length; i++) {
            printf("testing %s  \n", points.ids[i]);
			bidib_switch_point(points.ids[i], "reverse");
			state = bidib_get_point_state(points.ids[i]);
			logTestResult(test, state, i);

			}

		sleep(WAITINGTIME);
		for (int i = 0; i < points.length; i++) {
            printf("testing %s  \n", points.ids[i]);
			bidib_switch_point(points.ids[i], "normal");
			state = bidib_get_point_state(points.ids[i]);
			logTestResult(test, state, i);

			}

		sleep(WAITINGTIME);
	}


	void driveTo( char* segment, int speed, char* train){
		bidib_set_train_speed(train, speed, master);
		while(1) {
			pos = bidib_get_train_position(train);
					sleep(1);
			for (int i = 0; i < pos.length; i++) {
				if (!strcmp(pos.segments[i], segment)) {
						return;
				}
			}
	}

  void driveToStop( char* segment, int speed, char* train){
		bidib_set_train_speed(train, speed, master);
		while(1) {
			pos = bidib_get_train_position(train);
					sleep(1);
			for (int i = 0; i < pos.length; i++) {
				if (!strcmp(pos.segments[i], segment)) {
						bidib_set_train_speed(train, 0, master);
						return;
				}
			}
	}
