/*
 * (c) 2016 by Ricardo Branco
 * MIT License
 *
 * v1.1
 *
 * This program is the chmod equivalent to ipcmk(1) and ipcrm(1).
 *
 * Notes:
 *   + The mode must be specified in octal mode.
 *   + The 0x1000 bit sets the IPC_RMID flag.
 *     In the case of message queues and semaphores, it is equivalent to ipcrm(1).
 *     In the case of shared memory segments, sets it to be destroyed when the last process attached dies.
 *   + The 0x2000 bit sets the IPC_LOCK flag to lock the shared memory segment to RAM. This flag is Linux-only.
 *     The RLIMIT_MEMLOCK value (ulimit -l) from getrlimit(2) applies for non-privileged users.
 *   + See the msgctl(2), semctl(2) & shmctl(2) for details.
 *   + It supports IPC namespaces. (Linux only). The argument to the -i option is a PID.
 *     See lsns(1) from latest util-linux to list namespaces.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <errno.h>
#include <libgen.h>
#include <unistd.h>
#ifdef __linux__
#include <sys/types.h>
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
	fprintf(stderr, "Usage: %s [-i PID] [-m|-q|-s] MODE SHMID|MSQID|SEMID...\n", progname);
#else
	fprintf(stderr, "Usage: %s [-m|-q|-s] MODE SHMID|MSQID|SEMID...\n", progname);
#endif
	exit(status);
}

static int msgmod(char *argv[], unsigned short mode)
{
	struct msqid_ds info;
	int errors = 0;
	int id;

	while (*++argv != NULL) {
		id = (int) xstrtoul(*argv, 10);
		if (msgctl(id, IPC_STAT, &info) < 0) {
			fprintf(stderr, "%s: %d\n", strerror(errno), id);
			errors++;
			continue;
		}

		info.msg_perm.mode = mode;

		if (msgctl(id, IPC_SET, &info) < 0) {
			fprintf(stderr, "msgctl(IPC_SET): %d: %s\n", id, strerror(errno));
			errors++;
		}
	}

	return errors;
}

static int semmod(char *argv[], unsigned short mode)
{
	struct semid_ds info;
	int errors = 0;
	int id;

	while (*++argv != NULL) {
		id = (int) xstrtoul(*argv, 10);
		if (semctl(id, 0, IPC_STAT, &info) < 0) {
			fprintf(stderr, "%s: %d\n", strerror(errno), id);
			errors++;
			continue;
		}

		info.sem_perm.mode = mode;

		if (semctl(id, 0, IPC_SET, &info) < 0) {
			fprintf(stderr, "semctl(IPC_SET): %d: %s\n", id, strerror(errno));
			errors++;
		}
	}

	return errors;
}

static int shmmod(char *argv[], unsigned short mode)
{
	struct shmid_ds info;
	int errors = 0;
	void *addr;
	int cmd;
	int id;

	while (*++argv != NULL) {
		id = (int) xstrtoul(*argv, 10);
		if (shmctl(id, IPC_STAT, &info) < 0) {
			fprintf(stderr, "%s: %d\n", strerror(errno), id);
			errors++;
			continue;
		}

		info.shm_perm.mode = mode;

		if (shmctl(id, IPC_SET, &info) < 0) {
			fprintf(stderr, "shmctl(IPC_SET): %d: %s\n", id, strerror(errno));
			errors++;
		}

		if (mode & SHM_DEST) {
			if (shmctl(id, IPC_RMID, NULL) < 0) {
				fprintf(stderr, "shmctl(IPC_RMID): %d: %s\n", id, strerror(errno));
				errors++;
			}
		}

#ifdef SHM_LOCK
		if (mode & SHM_LOCKED)
			cmd = info.shm_perm.mode & SHM_LOCKED ? 0 : SHM_LOCK;
		else
			cmd = info.shm_perm.mode & SHM_LOCKED ? SHM_UNLOCK : 0;

		if (cmd) {
			addr = shmat(id, NULL, 0);
			if (addr == (void *) -1)
				/* shmat() may fail if we are the last process attached and SHM_RMID was set */
				continue;
			if (shmctl(id, cmd, NULL) < 0) {
				fprintf(stderr, "SHM_LOCK: %d: %s\n", id, strerror(errno));
				errors++;
			}
			(void) shmdt(addr);
		}
#endif
	}

	return errors;
}

int main(int argc, char *argv[])
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
			fprintf(stderr, "ERROR: invalid option: -%c\n", optopt);
			exit_usage(1);
		}
	}

	argc -= optind;
	argv += optind;

	if (argc < 2)
		exit_usage(1);

	mode = (unsigned short) xstrtoul(*argv, 8);
	if (mode > mask) {
		fprintf(stderr, "Invalid mode: %o\n", mode);
		exit(1);
	}

#ifdef CLONE_NEWIPC
	if (fd != -1 && setns(fd, CLONE_NEWIPC) < 0) {
		perror("setns()");
		exit(1);
	}
#endif

	exit(ipcmod(argv, mode & mask) ? 1: 0);
}

