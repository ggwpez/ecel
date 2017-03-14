#include "fail.h"

#include <stdio.h>
#include <stdlib.h>

int unalert = 0;

int fail(int minor, char const* format, ...)
{
	va_list args;
	va_start(args, format);

	char* ptr = 0;
	do
	{
		ptr	= va_arg(args, char*);
	} while (ptr);

	va_end(args);
	return vfail(minor, format, args);
}

int vfail(int minor, const char* format, va_list args)
{
	if (minor && unalert)
	{
		fprintf(stderr, "%s: ", "Ignoring error");
		vfprintf(stderr, format, args);
		fputs("\n", stderr);

		return 0;
	}
	else
	{
		vfprintf(stderr, format, args);
		if (minor)
			fputs("\nPass --unalert to ignore minor errors\n", stderr);
		else
			fputs("\n", stderr);

		exit(-1);
		return -1;
	}
}
