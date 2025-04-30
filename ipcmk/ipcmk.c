/* MIT license clone of GNU util-linux ipcmk */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <getopt.h>

#ifdef __FreeBSD__
#include <libutil.h>
#endif

#include "extern.h"

static void
usage(void)
{
	printf("Usage:\n");
	printf(" ipcmk [options]\n\n");
	printf("Create various IPC resources.\n\n");
	printf("Options:\n");
	printf(" -M, --shmem <size>       create shared memory segment of size <size>\n");
	printf(" -S, --semaphore <number> create semaphore array with <number> elements\n");
	printf(" -Q, --queue              create message queue\n");
	printf(" -p, --mode <mode>        permission for the resource (default is 0644)\n\n");
	printf(" -h, --help               display this help\n");
	printf(" -V, --version            display version\n\n");
	printf("Arguments:\n");
	printf(" <size> arguments may be followed by the suffixes for\n");
	printf("   GiB, TiB, PiB, EiB, ZiB, and YiB (the \"iB\" is optional)\n");
}

int
main(int argc, char *argv[]) {
	int id, nsems = 0, opt, perms = 0644;
	bool do_shm, do_sem, do_msg;
	size_t size = 0;
	key_t key;
	char *endp;

	do_shm = do_sem = do_msg = false;

	struct option long_options[] = {
		{"shmem",	required_argument,	0, 'M'},
		{"semaphore",	required_argument,	0, 'S'},
		{"queue",	no_argument,		0, 'Q'},
		{"mode",	required_argument,	0, 'p'},
		{"help",	no_argument,		0, 'h'},
		{"version",	no_argument,		0, 'V'},
		{0, 0, 0, 0}
	};

	while ((opt = getopt_long(argc, argv, "M:S:Qp:hV", long_options, NULL)) != -1) {
		switch (opt) {
		case 'M':
			if (expand_number(optarg, (uint64_t *) &size) == -1)
				err(1, "%s", optarg);
			do_shm = true;
			break;
		case 'S':
			nsems = atoi(optarg);
			do_sem = true;
			break;
		case 'Q':
			do_msg = true;
			break;
		case 'p':
			errno = 0;
			perms = strtoul(optarg, &endp, 8);
			if (errno || *endp != '\0')
				errx(1, "Invalid mode: %s", optarg);
			break;
		case 'h':
			usage();
			exit(EXIT_SUCCESS);
		case 'V':
			printf("ipcmk version 1.0\n");
			exit(EXIT_SUCCESS);
		default:
			usage();
			exit(EXIT_FAILURE);
		}
	}
	argc -= optind;
	argv += optind;

	key = arc4random();

	if (!do_shm && !do_sem && !do_msg) {
		warnx("bad usage");
		usage();
		exit(EXIT_FAILURE);
	}

	perms |= IPC_CREAT | IPC_EXCL;

	if (do_shm) {
		if ((id = shmget(key, size, perms)) == -1)
			err(1, "shmget");
		printf("Shared memory id: %d\n", id);
	}
       	if (do_sem) {
		if ((id = semget(key, nsems, perms)) == -1)
			err(1, "semget");
		printf("Semaphore id: %d\n", id);
	}
	if (do_msg) {
		if ((id = msgget(key, perms)) == -1)
			err(1, "msgget");
		printf("Message queue id: %d\n", id);
	}

	return (0);
}
