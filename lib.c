#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

unsigned long xstrtoul(const char *str, int base)
{
	int saved = errno;
	unsigned long x;
	char *tmp;

	errno = 0;
	x = strtoul(str, &tmp, base);
	if (errno) {
		perror(str);
		exit(1);
	}
	else if (tmp == str || *tmp != '\0') {
		fprintf(stderr, "Invalid number in given base: %s\n", str);
		exit(1);
	}

	errno = saved;

	return x;
}
