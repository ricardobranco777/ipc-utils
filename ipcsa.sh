#!/bin/bash

cat /proc/sysvipc/shm | \
while read key shmid perms size cpid lpid nattch uid gid cuid cgid atime dtime ctime rss swap ; do
	# Print header
	if [[ $key == "key" ]] ; then
		printf "%-8s %-8s perms\tcpid\tlpid\tc_cmdline l_cmdline\n" shmid size
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
	[ ! -d /proc/$lpid ] && lpid="!$lpid"

	printf "%-8d %-8d ${perms}\t${cpid}\t${lpid}\t$c_cmdline $l_cmdline\n" $shmid $size
done

