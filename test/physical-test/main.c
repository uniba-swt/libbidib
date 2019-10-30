#include <stdio.h>
#include <unistd.h>
#include <bidib.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#define WAITINGTIME 4


typedef struct {
    int stateError;
    int stateNotReached;
    int stateNotReachedVerified;
    int stateReached;
    int stateReachedVerified;
    int unknownState;

}Point_result;

typedef struct {
    Point_result* points;
}Test_result;

t_bidib_id_list_query points;
t_bidib_id_list_query signals;
t_bidib_unified_accessory_state_query state;
t_bidib_train_position_query pos;

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
		

    int i;

    if(bidib_start_serial("/dev/ttyUSB0", "../configurations/swtbahn-standard", 200)){

        printf("failed to start\n");
        return 0;
    }
    
	points = bidib_get_connected_points();
	signals = bidib_get_connected_signals();
	int t;
    Test_result *test = malloc(sizeof(Test_result));
    test->points = malloc(points.length * sizeof(Point_result));

    for(i = 0; i < points.length; i++){
        test->points[i].stateReachedVerified = 0;
        test->points[i].stateReached = 0;
        test->points[i].stateNotReachedVerified = 0;
        test->points[i].stateNotReached = 0;
        test->points[i].stateError = 0;
        test->points[i].unknownState = 0;
    }
    
    
    
		if(atoi(args[1]) == 1){
				rounds= atoi(args[2]);
				t = 0;
				while(t < rounds) {
					printf("Test case 1\n");
					for (i = 0; i < points.length; i++) {
						if(!strcmp(points.ids[i], "sync1")){
							
							}
						else if(!strcmp(points.ids[i], "sync2")){
							
							}
						else if(!strcmp(points.ids[i], "lanterns1")){
							
							}
						else if(!strcmp(points.ids[i], "lanterns2")){
							
							}
						else{
						printf("testing %s numTest %d\n", points.ids[i], t);
						bidib_switch_point(points.ids[i], "reverse");

						state = bidib_get_point_state(points.ids[i]);
						logTestResult(test, state, i);
						
						}
					}
					sleep(WAITINGTIME);
					for (i = 0; i < points.length; i++) {
						if(!strcmp(points.ids[i], "sync1")){
							
							}
						else if(!strcmp(points.ids[i], "sync2")){
							
							}
						else if(!strcmp(points.ids[i], "lanterns1")){
							
							}
						else if(!strcmp(points.ids[i], "lanterns2")){
							
							}
						else{
						printf("testing %s numTest %d\n", points.ids[i], t);
						bidib_switch_point(points.ids[i], "normal");

						state = bidib_get_point_state(points.ids[i]);
						logTestResult(test, state, i);
						
						}
					}
					sleep(WAITINGTIME);
				   t++;
				}
				for(i = 0; i < points.length; i++){
					if(!strcmp(points.ids[i], "sync1")){
							
							}
						else if(!strcmp(points.ids[i], "sync2")){
							
							}
						else if(!strcmp(points.ids[i], "lanterns1")){
							
							}
						else if(!strcmp(points.ids[i], "lanterns2")){
							
							}
							else{
						printf("Point: %s \n", points.ids[i]);
						printf("unknown state: %d \n", test->points[i].unknownState);
						printf("state reached: %d \n", test->points[i].stateReached);
						printf("state unreached: %d \n", test->points[i].stateNotReached);
						printf("state reached verified: %d \n", test->points[i].stateReachedVerified);
						printf("state unreached verified: %d \n", test->points[i].stateNotReachedVerified);
						printf("state error: %d \n", test->points[i].stateError);
						printf("\n");
							}

						}
			
			}
		else if(atoi(args[1]) == 2){
			rounds= atoi(args[2]);
			t = 0;
			while(t < rounds) {
			printf("Test case 1\n");
				for (i = 0; i < points.length; i++) {
					if(!strcmp(points.ids[i], "sync1")){
						
						}
					else if(!strcmp(points.ids[i], "sync2")){
						
						}
					else if(!strcmp(points.ids[i], "lanterns1")){
						
						}
					else if(!strcmp(points.ids[i], "lanterns2")){
						
						}
					else{
					printf("testing %s numTest %d\n", points.ids[i], t);
					bidib_switch_point(points.ids[i], "reverse");
					sleep(WAITINGTIME);
					state = bidib_get_point_state(points.ids[i]);
					logTestResult(test, state, i);
					
					bidib_switch_point(points.ids[i], "normal");
					sleep(WAITINGTIME);
					state = bidib_get_point_state(points.ids[i]);
					logTestResult(test, state, i);
					}
				}
				
			   t++;
				}
				for(i = 0; i < points.length; i++){
					if(!strcmp(points.ids[i], "sync1")){
							
							}
						else if(!strcmp(points.ids[i], "sync2")){
							
							}
						else if(!strcmp(points.ids[i], "lanterns1")){
							
							}
						else if(!strcmp(points.ids[i], "lanterns2")){
							
							}
							else{
						printf("Point: %s \n", points.ids[i]);
						printf("unknown state: %d \n", test->points[i].unknownState);
						printf("state reached: %d \n", test->points[i].stateReached);
						printf("state unreached: %d \n", test->points[i].stateNotReached);
						printf("state reached verified: %d \n", test->points[i].stateReachedVerified);
						printf("state unreached verified: %d \n", test->points[i].stateNotReachedVerified);
						printf("state error: %d \n", test->points[i].stateError);
						printf("\n");
							}

						}
			
			
			}
			
			if(atoi(args[1]) == 4){
				rounds= atoi(args[2]);
				t = 0;
				while(t < rounds) {
					printf("Test case 1\n");
					
						bidib_switch_point("point1", "reverse");
						sleep (1);

					bidib_switch_point("point9", "reverse");
					sleep (1);
					bidib_switch_point("point11", "reverse");
						sleep (1);

					bidib_switch_point("point10", "normal");
						sleep (1);

					bidib_switch_point("point5", "normal");
						sleep (1);

					bidib_switch_point("point7", "normal");
						sleep (1);


					bidib_switch_point("point2", "normal");
						sleep (1);

					bidib_switch_point("point3", "normal");
						sleep (1);

					bidib_switch_point("point6", "normal");
						sleep (1);
					bidib_switch_point("point1", "reverse");
						sleep (1);
							bidib_switch_point("point1", "reverse");
						sleep (1);
						printf("Points switched\n");

					int i;

					t_bidib_train_position_query pos;
					sleep(2);
					bidib_set_train_speed("regional_odeg", 60, "master");
					bidib_set_train_speed("cargo_bayern", 60, "master");
				sleep(5);

					while(1) {
							pos = bidib_get_train_position("cargo_bayern");
									sleep(1);
							for (i = 0; i < pos.length; i++) {
								if (!strcmp(pos.segments[i], "seg3")) {
					
									bidib_set_train_speed("cargo_bayern", 0, "master");
									i = -1;
									break;
								}
							}
							if (i == -1) {
								break;
							}
					}
					while(1) {
						pos = bidib_get_train_position("regional_odeg");
								sleep(1);
						for (i = 0; i < pos.length; i++) {
							if (!strcmp(pos.segments[i], "seg25")) {
								usleep(500);
							bidib_switch_point("point10", "reverse");
							bidib_switch_point("point5", "reverse");
							bidib_switch_point("point9", "reverse");
							bidib_switch_point("point11", "reverse");
							bidib_switch_point("point12", "reverse");

								i = -1;
								break;
							}
						}
						if (i == -1) {
							break;
						}
					}
					while(1) {
							pos = bidib_get_train_position("regional_odeg");
									sleep(1);
							for (i = 0; i < pos.length; i++) {
								if (!strcmp(pos.segments[i], "seg40")) {
									usleep(500);
									bidib_set_train_speed("regional_odeg", 0, "master");
							bidib_set_train_speed("cargo_bayern", 60, "master");
							
									i = -1;
									break;
								}
							}
							if (i == -1) {
								break;
							}
					}
					
					while(1) {
							pos = bidib_get_train_position("cargo_bayern");
									sleep(1);
							for (i = 0; i < pos.length; i++) {
								if (!strcmp(pos.segments[i], "seg12")) {

								bidib_switch_point("point6", "normal");
							bidib_switch_point("point2", "normal");
							bidib_switch_point("point8", "normal");
							
									i = -1;
									break;
								}
							}
							if (i == -1) {
								break;
							}
					}


					while(1) {
						pos = bidib_get_train_position("cargo_bayern");
								sleep(1);
						for (i = 0; i < pos.length; i++) {
							if (!strcmp(pos.segments[i], "seg3")) {

								bidib_set_train_speed("cargo_bayern", 0, "master");
								bidib_switch_point("point9", "reverse");
								bidib_switch_point("point11", "reverse");


								bidib_set_train_speed("regional_odeg", -60, "master");
						
								i = -1;
								break;
							}
						}
						if (i == -1) {
							break;
						}
					}

					while(1) {
							pos = bidib_get_train_position("regional_odeg");
									sleep(1);
							for (i = 0; i < pos.length; i++) {
								if (!strcmp(pos.segments[i], "seg37")) {
									usleep(500);
							bidib_set_train_speed("regional_odeg", 0, "master");
									i = -1;
									break;
								}
							}
							if (i == -1) {
								break;
							}
					}
						t++;
				}
			 }
			
			else if(atoi(args[1]) == 5){
				
					rounds= atoi(args[2]);
					t = 0;
					while(t < rounds) {
						
						for (i = 0; i < signals.length; i++) {
							printf("testing %s numTest %d\n", signals.ids[i], t);
							
							if(!strcmp(signals.ids[i], "signal19")){
								bidib_set_signal(signals.ids[i], "on");
							}
							else if(!strcmp(signals.ids[i], "signal17")){
								bidib_set_signal(signals.ids[i], "on");
								}
							else if(!strcmp(signals.ids[i], "signal1")){
								bidib_set_signal(signals.ids[i], "on");
								}
							else{
								bidib_set_signal(signals.ids[i], "red");
							}
						}
						sleep(WAITINGTIME);
						
						for (i = 0; i < signals.length; i++) {
							printf("testing %s numTest %d\n", signals.ids[i], t);
							
							if(!strcmp(signals.ids[i], "signal19")){
								bidib_set_signal(signals.ids[i], "off");
							}
							else if(!strcmp(signals.ids[i], "signal17")){
								bidib_set_signal(signals.ids[i], "off");
								}
							else if(!strcmp(signals.ids[i], "signal1")){
								bidib_set_signal(signals.ids[i], "off");
								}
							else{
								bidib_set_signal(signals.ids[i], "yellow");
							}
						}
						sleep(WAITINGTIME);
						
						for (i = 0; i < signals.length; i++) {
							printf("testing %s numTest %d\n", signals.ids[i], t);
							
							if(!strcmp(signals.ids[i], "signal19")){
								bidib_set_signal(signals.ids[i], "off");
							}
							else if(!strcmp(signals.ids[i], "signal17")){
								bidib_set_signal(signals.ids[i], "off");
								}
							else if(!strcmp(signals.ids[i], "signal1")){
								bidib_set_signal(signals.ids[i], "off");
								}
							else{
								bidib_set_signal(signals.ids[i], "green");
							}
						}
						
						sleep(WAITINGTIME);
					   t++;
					}
				}
				else if(atoi(args[1]) == 3){
					rounds= atoi(args[2]);
					t = 0;
					int i;
					while(t < rounds) {
							bidib_switch_point("point1", "normal");
							bidib_switch_point("point2", "normal");
							bidib_switch_point("point3", "normal");
							bidib_switch_point("point6", "reverse");
						
						while(1) {
								pos = bidib_get_train_position("cargo_bayern");
								sleep(1);
								for (i = 0; i < pos.length; i++) {
									if (!strcmp(pos.segments[i], "seg1")) {
										sleep(3);
										bidib_set_train_speed("cargo_bayern", 120, "master");
									
										
										i = -1;
										break;
										}
								}
								if (i == -1) {
									break;
								}
							}
						
						
							while(1) {
								pos = bidib_get_train_position("cargo_bayern");
								sleep(1);
								for (i = 0; i < pos.length; i++) {
									if (!strcmp(pos.segments[i], "seg13")) {
				
										bidib_set_train_speed("cargo_bayern", 60, "master");
										
										bidib_switch_point("point8", "reverse");
										bidib_switch_point("point2", "reverse");
										bidib_switch_point("point3", "reverse");
										bidib_switch_point("point4", "reverse");
										
										bidib_switch_point("point5", "normal");
										bidib_switch_point("point9", "normal");
										bidib_switch_point("point10", "normal");
										bidib_switch_point("point7", "normal");
										
										i = -1;
										break;
										}
								}
								if (i == -1) {
									break;
								}
							}
							
							while(1) {
								pos = bidib_get_train_position("cargo_bayern");
								sleep(1);
								for (i = 0; i < pos.length; i++) {
									if (!strcmp(pos.segments[i], "seg28")) {
				
										bidib_set_train_speed("cargo_bayern", 40, "master");
										

										bidib_switch_point("point4", "normal");
										bidib_switch_point("point10", "reverse");
										bidib_switch_point("point5", "reverse");
										
										bidib_switch_point("point12", "normal");
										bidib_switch_point("point9", "reverse");
										bidib_switch_point("point11", "reverse");
										
										i = -1;
										break;
										}
								}
								if (i == -1) {
									break;
								}
							}
							
							while(1) {
								pos = bidib_get_train_position("cargo_bayern");
								sleep(1);
								for (i = 0; i < pos.length; i++) {
									if (!strcmp(pos.segments[i], "seg37")) {
				
										bidib_set_train_speed("cargo_bayern", 5, "master");

										
										i = -1;
										break;
										}
								}
								if (i == -1) {
									break;
								}
							}
							
							
							while(1) {
								pos = bidib_get_train_position("cargo_bayern");
								sleep(1);
								for (i = 0; i < pos.length; i++) {
									if (!strcmp(pos.segments[i], "seg38")) {
										bidib_set_train_speed("cargo_bayern", 0, "master");
										bidib_set_train_speed("cargo_bayern", -40, "master");

										bidib_switch_point("point12", "reverse");
										i = -1;
										break;
										}
								}
								if (i == -1) {
									break;
								}
							}
							
							while(1) {
								pos = bidib_get_train_position("cargo_bayern");
								sleep(1);
								for (i = 0; i < pos.length; i++) {
									if (!strcmp(pos.segments[i], "seg40")) {
				
										bidib_set_train_speed("cargo_bayern", -5, "master");

										
										i = -1;
										break;
										}
								}
								if (i == -1) {
									break;
								}
							}
							
							while(1) {
								pos = bidib_get_train_position("cargo_bayern");
								sleep(1);
								for (i = 0; i < pos.length; i++) {
									if (!strcmp(pos.segments[i], "seg41")) {
				
										bidib_set_train_speed("cargo_bayern", 40, "master");
										

										
										i = -1;
										break;
										}
								}
								if (i == -1) {
									break;
								}
							}
							
							while(1) {
								pos = bidib_get_train_position("cargo_bayern");
								sleep(1);
								for (i = 0; i < pos.length; i++) {
									if (!strcmp(pos.segments[i], "seg37")) {
				
										bidib_set_train_speed("cargo_bayern", -40, "master");
										

										bidib_switch_point("point4", "normal");
										bidib_switch_point("point10", "reverse");
										bidib_switch_point("point5", "reverse");
										
										bidib_switch_point("point12", "normal");
										bidib_switch_point("point9", "reverse");
										bidib_switch_point("point11", "reverse");
										
										i = -1;
										break;
										}
								}
								if (i == -1) {
									break;
								}
							}
							
							while(1) {
								pos = bidib_get_train_position("cargo_bayern");
								sleep(1);
								for (i = 0; i < pos.length; i++) {
									if (!strcmp(pos.segments[i], "seg28")) {
				
										bidib_set_train_speed("cargo_bayern", -60, "master");
										
										bidib_switch_point("point8", "reverse");
										bidib_switch_point("point2", "reverse");
										bidib_switch_point("point3", "reverse");
										bidib_switch_point("point4", "reverse");
										
										bidib_switch_point("point5", "normal");
										bidib_switch_point("point9", "normal");
										bidib_switch_point("point10", "normal");
										bidib_switch_point("point7", "normal");
										
										i = -1;
										break;
										}
								}
								if (i == -1) {
									break;
								}
							}
							
							while(1) {
								pos = bidib_get_train_position("cargo_bayern");
								sleep(1);
								for (i = 0; i < pos.length; i++) {
									if (!strcmp(pos.segments[i], "seg13")) {
									bidib_switch_point("point1", "normal");
									bidib_switch_point("point2", "normal");
									bidib_switch_point("point3", "normal");
									bidib_switch_point("point6", "reverse");
									bidib_set_train_speed("cargo_bayern", -80, "master");
										
										
										i = -1;
										break;
										}
								}
								if (i == -1) {
									break;
								}
								}
								while(1) {
								pos = bidib_get_train_position("cargo_bayern");
								sleep(1);
								for (i = 0; i < pos.length; i++) {
									if (!strcmp(pos.segments[i], "seg4")) {

									bidib_set_train_speed("cargo_bayern", -5, "master");
										
										
										i = -1;
										break;
										}
								}
								if (i == -1) {
									break;
								}
								}
								while(1) {
								pos = bidib_get_train_position("cargo_bayern");
								sleep(1);
								for (i = 0; i < pos.length; i++) {
									if (!strcmp(pos.segments[i], "seg1")) {
										bidib_set_train_speed("cargo_bayern", 0, "master");
									
										
										i = -1;
										break;
										}
								}
								if (i == -1) {
									break;
								}
							}
							t++;
							}
						}
						if(atoi(args[1]) == 6){
							
							rounds= atoi(args[2]);
							int time = atoi(args[4]);
							time = time * 1000;
							int p = atoi(args[3]);
							t = 0;
							while(t < rounds) {
								printf("testing %s normal, numTest %d\n", points.ids[p], t);
								bidib_switch_point(points.ids[p], "normal");

								state = bidib_get_point_state(points.ids[p]);
								logTestResult(test, state, p);
								usleep(time);
								printf("testing %s reverse, numTest %d\n", points.ids[p], t);
								bidib_switch_point(points.ids[p], "reverse");

								state = bidib_get_point_state(points.ids[p]);
								logTestResult(test, state, p);
								usleep(time);
								t++;
							}
							
							printf("Point: %s \n", points.ids[p]);
							printf("unknown state: %d \n", test->points[p].unknownState);
							printf("state reached: %d \n", test->points[p].stateReached);
							printf("state unreached: %d \n", test->points[p].stateNotReached);
							printf("state reached verified: %d \n", test->points[p].stateReachedVerified);
							printf("state unreached verified: %d \n", test->points[p].stateNotReachedVerified);
							printf("state error: %d \n", test->points[p].stateError);
							printf("\n");
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
