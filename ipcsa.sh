#!/bin/bash

cat /proc/sysvipc/shm | \
while read key shmid perms size cpid lpid nattch uid gid cuid cgid atime dtime ctime rss swap ; do
	# Print header
	if [[ $key == "key" ]] ; then
		printf "%-8s %-8s %-10s %-6s cpid\tc_cmdline\n" "owner" "shmid" "size" "perms"
		continue
	fi

	c_cmdline=$(cat /proc/$cpid/cmdline 2>/dev/null | tr '\0' ' ' | sed 's/ *$//')
	c_cmdline=${c_cmdline:-$(cat /proc/$cpid/comm 2>/dev/null)}
	c_cmdline="[$c_cmdline]"
	#c_exe=$(readlink -f /proc/$cpid/exe)
	l_cmdline=$(cat /proc/$lpid/cmdline 2>/dev/null | tr '\0' ' ' | sed 's/ *$//')
	l_cmdline=${l_cmdline:-$(cat /proc/$lpid/comm 2>/dev/null)}
	[ -n "$l_cmdline" ] && l_cmdline="[$l_cmdline]"
	#l_exe=$(readlink -f /proc/$lpid/exe)

	[ ! -d /proc/$cpid ] && cpid="!$cpid"

	owner=$(id -un)

	printf "%-8s %-8d %-10d %-6s ${cpid}\t$c_cmdline $l_cmdline\n" "$owner" "$shmid" "$size" "$perms"
done

