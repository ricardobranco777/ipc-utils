#!/bin/bash
#
# Run commands on all available namespaces of a specific type (default: IPC)
#
# Example: nsenter.sh ipcs -a
#

if [ $# -lt 1 ] ; then
	echo "Usage: [NS_TYPE=ipc|mnt|net|pid|uts] ${0##*/} COMMAND [ARGUMENTS...]" >&2
	exit 1
fi

if [ $(id -u) -ne 0 ] ; then
	sudo=$(type -P sudo)
fi

NS_TYPE=${NS_TYPE:-"ipc"}

if [[ ! $NS_TYPE =~ ^(ipc|mnt|net|pid|uts)$ ]] ; then
	echo "Unsupported namespace: $NS_TYPE" >&2
	exit 1
fi

init=$($sudo stat -Lc '%i' /proc/1/ns/$NS_TYPE)

$sudo find /proc -regex "/proc/[0-9]*/task/[0-9]*/ns/$NS_TYPE" -printf '%p %l\n' 2>/dev/null | sed -r 's/[^0-9]+/ /g' | \
while read pid tid inode ; do
	[[ -z $inode || $inode -eq $init ]] && continue
	if [[ -z ${pids["$inode"]} ]] ; then
		opts=(--mount=/proc/$pid/task/$tid/ns/mnt)
		for ns in ipc net pid uts ; do
			opts+=(--$ns=/proc/$pid/task/$tid/ns/$ns)
		done
		echo "NS=$inode" >/dev/tty
		$sudo nsenter ${opts[@]} -- "$@"
		pids["$inode"]="$pid"
	fi
done

