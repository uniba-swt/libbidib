#include <stdio.h>
#include <unistd.h>
#include <bidib.h>
#include <stdlib.h>

void print_segment_state_query(t_bidib_segment_state_query seg_state_query) {
	if (seg_state_query.known) {
		printf("occ: %s", seg_state_query.data.occupied ? "yes" : "no");
		
		t_bidib_id_query id_query;
		for (size_t i = 0; i < seg_state_query.data.dcc_address_cnt; i++) {
			if (i == 0) {
				printf(" - ");
			} else if (i > 0) {
				printf(", ");
			}
			
			id_query = bidib_get_train_id(seg_state_query.data.dcc_addresses[i]);
			if (id_query.known) {
				printf("%s", id_query.id);
			} else {
				printf("unknown");
			}
			bidib_free_id_query(id_query);
		}

		printf("\n");
	}
}

int main(void) {
	if(bidib_start_serial("/dev/cu.usbserial-A907DHAS", "../config", 0)) {
		printf("Unable to start serial communication.\n");
	} else {
		while (true) {
			t_bidib_segment_state_query seq_query = bidib_get_segment_state("seg1");
			printf("seg1 ");
			print_segment_state_query(seq_query);
			bidib_free_segment_state_query(seq_query);
		
			sleep(1);
		}
	}
	
	return 0;
}

