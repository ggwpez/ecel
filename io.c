#include "io.h"
#include "fail.h"

#include <assert.h>
#include <stdio.h>
#include <unistd.h>

FILE* open_file(char const* path, char const* mode)
{
	assert(path && mode);
	FILE* file = fopen(path, mode);

	if (! file)
	{
		return fail(0, "File not found: '%s'\n", path), NULL;
	}
	else
		return file;
}

char* read_file(FILE* f, len_t* len)
{
	assert(f && len);
	char* buffer = (char*)malloc(BUFF_SIZE);
	len_t tread = 0;
	*len = 0;
	assert(buffer);

	size_t pos = ftello(f);

	while (tread = fread(buffer +*len, 1, BUFF_SIZE, f))
	{
		*len += tread;

		if (tread == BUFF_SIZE)
			assert(buffer = realloc(buffer, *len +BUFF_SIZE));
	}

	// TODO assert
	fseeko(f, pos, SEEK_SET);
	return buffer;
}

int is_not_seekable(FILE* f)
{
	return (f == stdin);
	//return (isatty(fileno(f)));
}

// Dosent work on stdin
ssize_t flen(FILE* f)
{
	size_t s, l;

	// Will fseeko fail?
	if (is_not_seekable(f))
		return -1;

	s = ftello(f);
	if (fseeko(f, 0, SEEK_END))
		return fail(0, "fseeko error"), -1;
	l = ftello(f);
	if (fseeko(f, s, SEEK_SET))
		return fail(0, "fseeko error"), -1;

	return l;
}

ssize_t fsplice(FILE* in, FILE* out, ssize_t len)
{
	char c;
	ssize_t l;
	len_t pos1 = ftello(in),
		  pos2 = ftello(out);

	for (l = 0; l < len; ++l)
	{
		if (((c = fgetc(in)) == EOF) && ferror(in))
			return fail(0, "File_in read error");
		if ((fputc(c, out) == EOF) && ferror(out))
			return fail(0, "File_out write error");
	}

	fseeko(in, pos1, SEEK_SET);
	fseeko(out, pos2, SEEK_SET);

	return l;
}

struct tm read_tm(FILE* file)
{
	assert(file);
	struct tm time;

	skip(file, '(');
	time.tm_year = read_uint(8, file);
	skip(file, '-');
	time.tm_mon = read_uint(4, file) -1;
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
