#include <stdio.h>

#include <kernel/serial.h>
#include <kernel/terminal.h>
#include <kernel/memory.h>
#include <limine.h>


extern "C" void kernel_main(void) {
	serial_initialize();
	terminal_initialize();
	
	getMemoryInfo();

	PhysicalMemoryManager* pm = pmm();
	pm->initialize();
	VirtualMemoryManager* vm = vmm();
	vm->initialize();	
	
	for (;;);
}