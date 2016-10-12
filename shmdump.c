/*
 * (c) 2016 by Ricardo Branco
 * MIT License
 *
 * v1.1
 *
 * This programs dumps the memory specified by the SysV IPC Shared Memory ID to the specified file.
 *
 * Note:
 * It supports IPC namespaces. (Linux only). The argument to the -i option is a PID.
 * See lsns(1) from latest util-linux to list namespaces.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/shm.h>
#ifdef __linux__
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <sched.h>
#endif

unsigned long xstrtoul(const char *str, int base);

static char *progname;

static void exit_usage(int status)
{
#ifdef CLONE_NEWIPC
	printf("Usage: %s [-i PID] SHMID DUMPFILE\n", progname);
#else
	printf("Usage: %s SHMID DUMPFILE\n", progname);
#endif
	exit(status);
}

int main(int argc, char *argv[])
{
	struct shmid_ds info;
	void *addr;
	char *file;
	FILE *out;
	int opt;
	int id;
#ifdef CLONE_NEWIPC
	char path[PATH_MAX];
	pid_t fd = -1;
#endif

	progname = basename(strdup(argv[0]));

#ifdef CLONE_NEWIPC
	while ((opt = getopt(argc, argv, ":hi:")) != -1) {
#else
	while ((opt = getopt(argc, argv, ":h")) != -1) {
#endif
		switch (opt) {
		case 'h':
			exit_usage(0);
#ifdef CLONE_NEWIPC
		case 'i':
			if (fd != -1)
				exit_usage(1);
			(void) snprintf(path, PATH_MAX, "/proc/%ld/ns/ipc", (long) xstrtoul(optarg, 10));
			fd = open(path, O_RDONLY);
			if (fd < 0) {
				perror(path);
				exit(1);
			}
			break;
#endif
		default:
			exit_usage(1);
		}
	}

	argc -= optind;
	argv += optind;

	if (argc < 2)
		exit_usage(1);

	id = atoi(argv[0]);
	file = argv[1];

#ifdef CLONE_NEWIPC
	if (fd != -1 && setns(fd, CLONE_NEWIPC) < 0) {
		perror("setns()");
		exit(1);
	}
#endif

	if (shmctl(id, IPC_STAT, &info) < 0) {
		perror(argv[0]);
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

	(void) fwrite(addr, info.shm_segsz, 1, out);

	(void) shmdt(addr);

	exit(0);
}

