#include <sys/types.h>
#include <sys/shm.h>
#ifdef __linux__
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <sched.h>
#endif

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Get a sane version of basename on Linux */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <libgen.h>

static char *progname;

static void
exit_usage(int status)
{
	FILE *fp = status ? stderr : stdout;

#ifdef CLONE_NEWIPC
	fprintf(fp, "Usage: %s [-i PID] SHMID DUMPFILE\n", progname);
#else
	fprintf(fp, "Usage: %s SHMID DUMPFILE\n", progname);
#endif
	exit(status);
}

int
main(int argc, char *argv[])
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
			break;
#ifdef CLONE_NEWIPC
		case 'i':
			if (fd != -1)
				exit_usage(1);
			(void) snprintf(path, PATH_MAX, "/proc/%d/ns/ipc", atoi(optarg));
			if ((fd = open(path, O_RDONLY)) < 0)
				err(1, "%s", path);
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
	if (fd != -1 && setns(fd, CLONE_NEWIPC) < 0)
		err(1, "%s", "setns()");
#endif

	if (shmctl(id, IPC_STAT, &info) < 0)
		err(1, "%s", argv[0]);

	if ((out = fopen(file, "w+")) == NULL)
		err(1, "%s", file);

	if ((addr = shmat(id, NULL, 0)) == (void *) -1)
		err(1, "%s", "shmat()");

	(void) fwrite(addr, info.shm_segsz, 1, out);

	(void) shmdt(addr);

	exit(0);
}

