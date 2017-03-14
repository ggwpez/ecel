#include "fail.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int unalert = 0;

int fail(int minor_failure, char const* format, ...)
{
	assert(format);

	va_list args;
	va_start(args, format);

	char* ptr = 0;
	do
	{
		ptr	= va_arg(args, char*);
	} while (ptr);

	va_end(args);
	return vfail(minor_failure, format, args);
}

int vfail(int minor_failure, const char* format, va_list args)
{
	assert(format && args);

	if (minor_failure && unalert)
	{
		fprintf(stderr, "%s: ", "Ignoring error");
		vfprintf(stderr, format, args);
		fputs("\n", stderr);

		return 0;
	}
	else
	{
		vfprintf(stderr, format, args);
		if (minor_failure)
			fputs("\nPass --unalert to ignore minor errors\n", stderr);
		else
			fputs("\n", stderr);

		exit(EXIT_FAILURE);
	}
}
