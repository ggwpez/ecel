#include "io.h"

struct tm read_tm(FILE* file)
{
    struct tm time;
    return time;
}

int write_tm(struct tm const* time, FILE* file)
{
    return 0;
}

uint8_t read_uint8_t(FILE* file)
{
    uint8_t ret = 0;
    
    for (size_t i = 0; i < sizeof(uint8_t) << 1; ++i)
    {
	char c = fgetc(file);
	if (c == EOF)
	{
	    fputs("Unawaited EOF", stderr);
	    exit(-1);
	}
	else if (! isxdigit(c))
	{
	    fprintf(stderr, "Currently only works with hex input but got: '%c'", c);
	    exit(-1);
	}
	else
	    ret = (ret << 4) | ((c -'0') & 15);
    }

    return ret;
}

int write_uint8_t(uint8_t v, FILE* file)
{
    if (fprintf(file, "%*.*" PRIx8, 2, 2, v) != 2)
    {
	fputs("Could not fprintf", stderr);
	return 1;
    }
    
    return 0;

}

uint64_t read_uint64_t(FILE* file)
{
    uint64_t ret = 0;
    
    for (size_t i = 0; i < sizeof(uint64_t) << 1; ++i)
    {
	char c = fgetc(file);
	if (c == EOF)
	{
	    fputs("Unawaited EOF", stderr);
	    exit(-1);
	}
	else if (! isxdigit(c))
	{
	    fprintf(stderr, "Currently only works with hex input but got: '%c'", c);
	    exit(-1);
	}
	else
	    ret = (ret << 4) | ((c -'0') & 15);
    }

    return ret;
}

int write_uint64_t(uint64_t v, FILE* file)
{
    if (fprintf(file, "%*.*" PRIx64, 16, 16, v) != 16)
    {
	fputs("Could not fprintf", stderr);
	return 1;
    }
    
    return 0;
}
