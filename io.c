#include "io.h"
#include "fail.h"

#include <assert.h>

FILE* open_file(char const* path, char const* mode)
{
	assert(path && mode);
	FILE* file = fopen(path, mode);

	if (! file)
	{
		fclose(file);
		return fail(0, "File not found: '%s'\n", path), NULL;
	}
	else
		return file;
}

struct tm read_tm(FILE* file)
{
	assert(file);
	struct tm time;

	skip(file, '(');
	time.tm_year = read_uint(8, file);
	skip(file, '-');
	time.tm_mon = read_uint(4, file);
	skip(file, '-');
	time.tm_mday = read_uint(8, file);
	skip(file, ')');

	return time;
}

int write_tm(struct tm const* time, FILE* file)
{
	assert(time && file);
	assert(time->tm_year <= 255 && "Hello Mr. Future");

	if (fputc('(', file) == EOF
		|| write_uint(time->tm_year, 8, file)
		|| fputc('-', file) == EOF
		|| write_uint(time->tm_mon +1, 4, file)		// month is 0-11
		|| fputc('-', file) == EOF
		|| write_uint(time->tm_mday, 8, file)
		|| fputc(')', file) == EOF)
	{
		return fail(0, "Error serializing the time");
	}

	return 0;
}

#define IHEX(c) ((c) <= '9' ? (c) -'0' : (c) -'a' +10)

uint64_t read_uint(int const bits, FILE* file)
{
	assert(file);
	assert(bits && !(bits % 4) && bits <= 64);
	uint64_t ret = 0;

	for (size_t i = 0; i < (bits >> 2); ++i)
	{
		char c = fgetc(file);

		if (c == EOF)
			return fail(0, "Unawaited EOF");
		else if (! isxdigit(c))
			return fail(0, "Currently only works with hex input but got: '%c'", c);
		else
			ret = (ret << 4) | IHEX(c);
	}

	return ret;
}

#define HEX(c) ((c) <= 9 ? '0' +(c) : 'a' +(c) -10)

int write_uint(uint64_t const v, int const bits, FILE* file)
{
	assert(file);
	assert(bits && !(bits % 4) && bits <= 64);

	for (int i = (bits >> 2) -1; i >= 0; --i)
		fputc(HEX((v >> (i << 2)) & 15), file);

	return 0;
}
