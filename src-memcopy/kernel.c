#include "io.h"

// Very very simple 'kernel' to dump memory over UART
unsigned long long *HEAP_START_;

void main()
{
	uart_init();
	unsigned char *HEAP_START = (unsigned char *)0x400000;	// Top of stack
	unsigned long HEAP_SIZE = 0x100000000 / 8;

	if ((long)HEAP_START % 8 != 0) {
		HEAP_START += 8 - ((long)HEAP_START % 8);
	}

	HEAP_START_ = (unsigned long long *)HEAP_START;

	int b = 0;
	for (long i = 0; i < HEAP_SIZE; i++) {
		char *bytes = (char*)&HEAP_START_[i];
		for(b = 0; b < 8; b++)
			uart_writeByteBlocking(bytes[b]);		
	}
}
