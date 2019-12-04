

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
