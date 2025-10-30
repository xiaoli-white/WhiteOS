#include <stdio.h>

#include <kernel/serial.h>
#include <kernel/memory.h>

extern "C" void kernel_main(void) {
	serial_init();
	printf("Hello, World!\n");
	
	get_memory_info();

	PMM* pmm = getPMM();
	pmm->init();
	
	for (;;);
}