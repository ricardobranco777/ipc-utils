#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#ifdef __linux__
#include <sys/types.h>
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
#define _GNU_SOURCE
#include <libgen.h>

static char *progname;

static void
exit_usage(int status)
{
	FILE *fp = status ? stderr : stdout;

#ifdef CLONE_NEWIPC
	fprintf(fp, "Usage: %s [-i PID] -m|-q|-s MODE SHMID|MSQID|SEMID...\n", progname);
#else
	fprintf(fp, "Usage: %s -m|-q|-s MODE SHMID|MSQID|SEMID...\n", progname);
#endif
	exit(status);
}

static int
msgmod(char *argv[], unsigned short mode)
{
	struct msqid_ds info;
	int errors = 0;
	int id;

	while (*++argv != NULL) {
		id = atoi(*argv);
		if (msgctl(id, IPC_STAT, &info) < 0) {
			warn("IPC_STAT: %d", id);
			errors++;
			continue;
		}

		info.msg_perm.mode = mode;

		if (msgctl(id, IPC_SET, &info) < 0) {
			warn("IPC_SET: %d", id);
			errors++;
		}
	}

	return errors;
}

static int
semmod(char *argv[], unsigned short mode)
{
	struct semid_ds info;
	int errors = 0;
	int id;

	while (*++argv != NULL) {
		id = atoi(*argv);
		if (semctl(id, 0, IPC_STAT, &info) < 0) {
			warn("IPC_STAT: %d", id);
			errors++;
			continue;
		}

		info.sem_perm.mode = mode;

		if (semctl(id, 0, IPC_SET, &info) < 0) {
			warn("IPC_SET: %d", id);
			errors++;
		}
	}

	return errors;
}

static int
shmmod(char *argv[], unsigned short mode)
{
	struct shmid_ds info;
	int errors = 0;
	int id;

	while (*++argv != NULL) {
		id = atoi(*argv);
		if (shmctl(id, IPC_STAT, &info) < 0) {
			warn("IPC_STAT: %d", id);
			errors++;
			continue;
		}

		info.shm_perm.mode = mode;

		if (shmctl(id, IPC_SET, &info) < 0) {
			warn("IPC_SET: %d", id);
			errors++;
		}

#ifdef SHM_LOCKED
		if (mode & SHM_DEST) {
			if (shmctl(id, IPC_RMID, NULL) < 0) {
				warn("IPC_RMID: %d", id);
				errors++;
			}
		}

		int cmd;
		if (mode & SHM_LOCKED)
			cmd = info.shm_perm.mode & SHM_LOCKED ? 0 : SHM_LOCK;
		else
			cmd = info.shm_perm.mode & SHM_LOCKED ? SHM_UNLOCK : 0;

		if (cmd) {
			void *addr;

			/* shmat() may fail if we are the last process attached and SHM_RMID was set */
			if ((addr = shmat(id, NULL, 0)) == (void *) -1)
				continue;
			if (shmctl(id, cmd, NULL) < 0) {
				warn("SHM_LOCK: %d", id);
				errors++;
			}
			(void) shmdt(addr);
		}
#endif
	}

	return errors;
}

int
main(int argc, char *argv[])
{
	int (*ipcmod)(char *argv[], unsigned short) = NULL;
	unsigned short mode;
	int mask = 0777;
	int opt;
#ifdef CLONE_NEWIPC
	char path[PATH_MAX];
	pid_t fd = -1;
#endif

	progname = basename(strdup(argv[0]));

#ifdef CLONE_NEWIPC
	while ((opt = getopt(argc, argv, ":hi:mqs")) != -1) {
#else
	while ((opt = getopt(argc, argv, ":hmqs")) != -1) {
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
		case 'm':
			if (ipcmod != NULL)
				exit_usage(1);
			ipcmod = &shmmod;
			mask = 03777;
			break;
		case 's':
			if (ipcmod != NULL)
				exit_usage(1);
			ipcmod = &semmod;
			break;
		case 'q':
			if (ipcmod != NULL)
				exit_usage(1);
			ipcmod = &msgmod;
			break;
		default:
			warn("Invalid option: -%c", optopt);
			exit_usage(1);
		}
	}

	if (ipcmod == NULL)
		exit_usage(1);

	argc -= optind;
	argv += optind;

	if (argc < 2)
		exit_usage(1);

	if ((mode = (unsigned short) strtoul(*argv, NULL, 8)) > mask)
		errx(1, "Invalid mode: %o", mode);

#ifdef CLONE_NEWIPC
	if (fd != -1 && setns(fd, CLONE_NEWIPC) < 0)
		err(1, "%s", "setns()");
#endif

	exit(ipcmod(argv, mode & mask) ? 1: 0);
}

