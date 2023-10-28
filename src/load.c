#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define IMAGES 5100U
#define SIZE 725444U

int main()
{
	char *buffer = malloc(SIZE);
	FILE *fp = fopen("mona.tga", "rb");
	fread(buffer, SIZE, 1, fp);

	char *buffer2 = malloc(SIZE * IMAGES);
	int i = 0;
	for (i = 0; i < IMAGES; i++) {
		printf("%u\n", (i * SIZE));
		memcpy(buffer2 + (i * SIZE), buffer, SIZE);
	}

	printf("Done\n");
	fflush(stdout);

	while (1) {
		sleep(5);
	}

	fclose(fp);
}
