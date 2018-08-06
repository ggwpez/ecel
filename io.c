#include "io.h"
#include "fail.h"
#include "defines.h"

#include <stdio.h>
#include <unistd.h>

FILE* open_file(char const* path, char const* mode)
{
	assert(path && mode);
	FILE* file = fopen(path, mode);

	if (! file)
	{
		fail(0, "File not found: '%s'\n", path);
		return NULL;
	}
	else
		return file;
}

char* read_file(FILE* f, len_t* len)
{
	*len = 0;
	assert(f && len);
	char* buffer = (char*)malloc(BUFF_SIZE);
	len_t tread = 0;
	*len = 0;
	assert(buffer);

	len_t pos = is_not_seekable(f) ? -1 : ftello(f);

	while ((tread = (len_t)fread(buffer +*len, 1, BUFF_SIZE, f)))
	{
		*len += tread;

		if (tread == BUFF_SIZE)
			assert(buffer = realloc(buffer, (size_t)*len +BUFF_SIZE));
	}

	if (pos != -1)
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
len_t flen(FILE* f)
{
	len_t s, l;

	// Will fseeko fail?
	if (is_not_seekable(f))
		return -1;

	s = ftello(f);
	if (fseeko(f, 0, SEEK_END))
	{
		fail(0, "fseeko error");
		return LEN_INVALID;
	}
	l = ftello(f);
	if (fseeko(f, s, SEEK_SET))
	{
		fail(0, "fseeko error");
		return LEN_INVALID;
	}

	return l;
}

len_t fsplice(FILE* in, FILE* out, len_t len)
{
	len_t l = 0;
	len_t pos1 = ftello(in),
		  pos2 = ftello(out);
	char* buffer = malloc(BUFF_SIZE);

	while (l < len)
	{
		size_t to_read = (len -l) > BUFF_SIZE ? BUFF_SIZE : (size_t)(len -l);

		if (fread(buffer, 1, to_read, in) != to_read)
			return fail(0, "File_in read error");
		if (fwrite(buffer, 1, to_read, out) != to_read)
			return fail(0, "File_out write error");

		l += to_read;
	}

	fseeko( in, pos1, SEEK_SET);
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

uint64_t read_uint(unsigned const bits, FILE* file)
{
	assert(file);
	assert(bits && !(bits % 4) && bits <= 64);
	uint64_t ret = 0;

	for (size_t i = 0; i < (bits >> 2); ++i)
	{
		int c = fgetc(file);

		if (c == EOF)
			return (uint64_t)fail(0, "Unawaited EOF");
		else if (! isxdigit(c))
			return (uint64_t)fail(0, "Currently only works with hex input but got: '%c'", c);
		else
			ret = (ret << 4) | (uint64_t)IHEX(c);
	}

	return ret;
}

#define HEX(c) ((c) <= 9 ? '0' +(c) : 'a' +(c) -10)

int write_uint(uint64_t const v, unsigned const bits, FILE* file)
{
	assert(file);
	assert(bits && !(bits % 4) && bits <= 64);

	for (int i = (bits >> 2) -1; i >= 0; --i)
		fputc(HEX((v >> (i << 2)) & 15), file);

	return 0;
}

int64_t read_int(int const bits, FILE* file)
{
	return (int64_t)read_uint(bits, file);
}

int write_int(int64_t const v, const unsigned bits, FILE* file)
{
	return write_uint((uint64_t const)v, bits, file);
}

len_t read_lent(FILE* file)
{
	assert(file);
	len_t ret = 0;

	for (size_t i = 0; i < 16; ++i)
	{
		int c = fgetc(file);

		if (c == EOF)
			return fail(0, "Unawaited EOF");
		else if (! isxdigit(c))
			return fail(0, "Currently only works with hex input but got: '%c'", c);
		else
			ret = (ret << 4) | IHEX(c);
	}

	return ret;
}

int write_lent(len_t const v, FILE* file)
{
	assert(file);

	for (int i = 16 -1; i >= 0; --i)
		fputc(HEX((v >> (i << 2)) & 15), file);

	return 0;
}
