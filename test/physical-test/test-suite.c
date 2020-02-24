

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

t_bidib_id_list_query sortOutIds(t_bidib_id_list_query idQuery, sortOutIds soid ){
		t_bidib_id_list_query iq;

		for (int i = 0; i < idQuery.length; i++) {
			for(int j = 0; j < soid.length; j++ )
					if(strcmp(points.ids[i], soid.ids[j])){

					}
				}

	void testSignal(t_bidib_id_list_query signals);

	void testPoint(t_bidib_id_list_query points, Test_result* test);

	void driveTo(t_bidib_train_position_query *pos, char** segment);
