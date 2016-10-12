#include <stdlib.h>
#include <errno.h>
#include <err.h>

unsigned long xstrtoul(const char *str, int base)
{
	int saved = errno;
	unsigned long x;
	char *tmp;

	errno = 0;
	x = strtoul(str, &tmp, base);
	if (errno)
		err(1, "%s", str);
	else if (tmp == str || *tmp != '\0')
		errx(1, "Invalid number in given base: %s", str);

	errno = saved;

	return x;
}
