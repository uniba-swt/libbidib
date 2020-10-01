#include <stdio.h>

#include <unistd.h>

#include <bidib.h>

#include "test-suite.h"

#include <stdlib.h>

#include <string.h>

#include <signal.h>


extern t_bidib_id_list_query points;
extern t_bidib_id_list_query signals;
extern t_bidib_unified_accessory_state_query state;
extern t_bidib_train_position_query pos;

int validateArg(int argc, char ** args);
void writeName();

void signal_callback_handler(int signum) {
    bidib_free_id_list_query(points);
    bidib_free_id_list_query(signals);
    bidib_free_unified_accessory_state_query(state);
    bidib_free_train_position_query(pos);
    bidib_set_train_speed("cargo_bayern", 0, "master");
    bidib_stop();
    printf("SIGINT - stopping libbidib \n");
    exit(signum);

}

int main(int argc, char ** args) {

    writeName();

    signal(SIGINT, signal_callback_handler);

    if (!validateArg(argc, args)) {
        printf("Usage: ./test-suite testCaseNum rounds [train] \n Cases:\n 1- Points paralell\n 2- Points serial \n 3- Track coverage (needs train arg) \n 5- Signals paralell \n");
        return 0;
    }

    if (bidib_start_serial("/dev/ttyUSB0", "../../../../swtbahn-cli/configurations/swtbahn-standard", 200)) {

        printf("failed to start\n");
        return 0;
    }

    t_testsuite_test_result * test = testsuite_initTestSuite();
    int rounds = 0, t = 0;
    rounds = atoi(args[2]);

    printf("Test case %d\n", atoi(args[1]));

    if (atoi(args[1]) == 1) {
        while (t < rounds) {
            testsuite_case_pointSerial(test);
            t++;
        }
        testsuite_printTestResults(test);
    } else if (atoi(args[1]) == 2) {
        while (t < rounds) {
            testsuite_case_pointParallel(test);
            t++;
        }
        testsuite_printTestResults(test);
    } else if (atoi(args[1]) == 3) {
        while (t < rounds) {
            testsuite_case_swtbahnStandardTrackCoverage(args[3]);
            t++;
        }
    } else if (atoi(args[1]) == 4) {
        //placeholder Case
    } else if (atoi(args[1]) == 5) {
        while (t < rounds) {
            testsuite_case_signal();
            t++;
        }
    }

    bidib_free_train_position_query(pos);
    bidib_free_id_list_query(signals);
    bidib_free_id_list_query(points);
    bidib_free_unified_accessory_state_query(state);
    free(test);

    bidib_stop();

    return 0;
}

int validateArg(int argc, char ** args) {
    if (argc < 3) {
        return 0;
    } else if ((argc > 3) && !(atoi(args[1]) == 3)) {
        return 0;
    } else if ((argc != 4) && (atoi(args[1]) == 3)) {
        return 0;
    } else if (!(atoi(args[1]) < 6)) {
        return 0;
    }

    return 1;
}

void writeName() {
    printf("************************\n"
		   "*                      *\n"
           "*  SWTbahn-test-suite  *\n"
           "*                      *\n"
           "************************\n"
           "*    UniBa-SWT-2020    *\n"
           "************************\n"
           "\n\n"
    );
}
