#include "helper.h"
#include "fail.h"

#include <stdlib.h>
#include <limits.h>
#include <errno.h>

uint64_t _strtoull(char const* ptr, int base)
{
	char* end;
	uint64_t ret = strtoull(ptr, &end, base);

	int err = errno;
	if ((end == ptr) || ((ret == ULLONG_MAX || ret == 0) && err == ERANGE) || (!ret && err == EINVAL))
		fail(0, "Expected an uint64_t but got '%s'", ptr);
	else
		return ret;
}

uint32_t _strtoul(char const* ptr, int base)
{
	char* end;
	uint32_t ret = strtoul(ptr, &end, base);

	int err = errno;
	if ((end == ptr) || ((ret == ULLONG_MAX || ret == 0) && err == ERANGE) || (!ret && err == EINVAL))
		fail(0, "Expected an uint32_t but got '%s'", ptr);
	else
		return ret;
}
