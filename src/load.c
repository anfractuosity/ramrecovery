#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// https://stackoverflow.com/questions/2513505/how-to-get-available-memory-c-g
unsigned long long getTotalSystemMemory()
{
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGE_SIZE);
    return pages * page_size;
}

int main()
{
	// Load image
	FILE *fp = fopen("mona.tga", "rb");

	// Get size of Mona file
	fseek(fp, 0L, SEEK_END);
	int monasize = ftell(fp);
	rewind(fp);

	// Read image to buffer
	char *buffer = malloc(monasize);
	fread(buffer, monasize, 1, fp);

	// Try to guess how much memory is free and make sure to subtract
	// an amount to avoid getting OOM killed (200MB in this case).
	unsigned long size = getTotalSystemMemory() - (1024 * 1024 * 200);
	char *buffer2 = NULL;

	// Try amounts decreasing by 50MB at a time, till we are able to allocate
	// memory (or stop attempting at 512MB)
	for (; size >= 1024 * 1024 * 512; size -= 1024 * 1024 * 50) {
		printf("Attempting to allocate %lu bytes\n", size);
		buffer2 = malloc(size);
		if (buffer2 != NULL)
			break;
	}

	if (buffer2) {
		// Fill memory with Mona Lisa
		long i = 0;
		while ((i * monasize) + monasize < size) {
			printf("%lu\n", (i * monasize));
			memcpy(buffer2 + (i * monasize), buffer, monasize);
			i++;
		}

		printf("Done\n");
		fflush(stdout);

		while (1) {
			sleep(5);
		}
	} else
		fprintf(stderr, "Failed\n");

	fclose(fp);
}
