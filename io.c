#include "io.h"
#include "fail.h"

#include <assert.h>

struct tm read_tm(FILE* file)
{
	struct tm time;
	return time;
}

int write_tm(struct tm const* time, FILE* file)
{
	return 0;
}

#define IHEX(c) ((c) <= '9' ? (c) -'0' : (c) -'a' +10)

uint64_t read_uint(int bits, FILE* file)
{
	uint64_t ret = 0;
	assert(bits && !(bits % 4) && bits <= 64);

	for (size_t i = 0; i < (bits >> 2); ++i)
	{
		char c = fgetc(file);
		if (c == EOF)
		{
			return fail(0, "Unawaited EOF");
		}
		else if (! isxdigit(c))
		{
			return fail(0, "Currently only works with hex input but got: '%c'", c);
		}
		else
		{
			int conv = IHEX(c);
			ret = (ret << 4) | conv;
		}
	}

	return ret;
}

#define HEX(c) ((c) <= 9 ? '0' +(c) : 'a' +(c) -10)

int write_uint(uint64_t v, int bits, FILE* file)
{
	assert(bits && !(bits % 4) && bits <= 64);
	int nibbles = bits >> 2;

	/*if (fprintf(file, "%*.*" PRIx64, nibbles, nibbles, v) != nibbles)
	{
		fputs("Could not fprintf", stderr);
		return 1;
	}*/
	for (int i = nibbles -1; i >= 0; --i)
	fputc(HEX((v >> (i << 2)) & 15), file);

	return 0;
}
