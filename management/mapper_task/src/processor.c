#include "processor.h"
#include "pkg.h"

void processor_init(processor_t *processors)
{
	// Echo("Initializing "); Echo(itoa(PKG_N_PE)); Echo(" processors\n");
	for(int i = 0; i < PKG_N_PE; i++){
		processors[i].addr = i / PKG_N_PE_X << 8 | i % PKG_N_PE_X;
		// Echo("Addr "); Echo(itoa(processors[i].addr));
		processors[i].free_page_cnt = PKG_MAX_LOCAL_TASKS;
		processors[i].pending_map_cnt = 0;
	}

	/* Mapper task temporarily only mapped to 0x0 */
	// processors[0].free_page_cnt--;
}

int processors_get_first_most_free(processor_t *processors, int old_proc)
{
	int address = 0;
	for(int i = 1; i < PKG_N_PE; i++){
		if(processors[i].free_page_cnt + (i == old_proc) > processors[address].free_page_cnt + (address == old_proc))
			address = i;
	}
	return address;
}
