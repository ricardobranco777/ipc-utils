/*
 * (c) 2016 by Ricardo Branco
 * MIT License
 *
 * v1.0
 *
 * This programs dumps the memory specified by the SysV IPC Shared Memory ID to the specified file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>

int main(int argc, char *argv[])
{
	struct shmid_ds info;
	void *addr;
	char *file;
	FILE *out;
	int id;

	if (argc != 3) {
		printf("Usage: %s SHMID DUMPFILE\n", argv[0]);
		exit(1);
	}

	id = atoi(argv[1]);
	file = argv[2];	

	if (shmctl(id, IPC_STAT, &info) < 0) {
		perror(argv[1]);
		exit(1);
	}

	out = fopen(file, "w+");
	if (out == NULL) {
		perror(file);
		exit(1);
	}

	addr = shmat(id, NULL, 0);
	if (addr == (void *) -1) {
		perror("shmat()");
		exit(1);
	}
	
	fwrite(addr, info.shm_segsz, 1, out);

	shmdt(addr);

	exit(0);
}

