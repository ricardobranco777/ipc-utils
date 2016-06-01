# ipc-utils
SysV IPC utilities that complement ipcmk and ipcrm

# ipcmod
This program is the chmod(1) equivalent to ipcmk(1), ipcs(1) & ipcrm(1)

# ipcsa.sh
A better ipcs -a

Notes:
- The mode must be specified in octal mode.
- The 0x1000 bit sets the IPC_RMID flag.
- In the case of message queues and semaphores, it is equivalent to ipcrm(1).
- In the case of shared memory segments, sets it to be destroyed when the last process attached dies.
- The 0x2000 bit sets the IPC_LOCK flag to lock the shared memory segment to RAM. This flag is Linux-only.
- The RLIMIT_MEMLOCK value (ulimit -l) from getrlimit(2) applies for non-privileged users.
- See the msgctl(2), semctl(2) & shmctl(2) for details.

# shmdump
Dumps the memory specified by the SysV IPC Shared Memory ID to the specified file.
