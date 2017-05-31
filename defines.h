#pragma once

#include "fail.h"

#include <inttypes.h>
#include <stddef.h>

// 1 MB
#define BUFF_SIZE (1 << 20)

typedef uint64_t kid_t;
typedef uint64_t len_t;
_Static_assert(sizeof(char) == 1, "sizeof(char) != 1");
_Static_assert(sizeof(len_t) >= sizeof(size_t), "sizeof(len_t) < sizeof(size_t)");
#define skip(f, c) { char got; if ((got = fgetc(f)) != (c)) { fail(0, "Syntax error\nAwaited: '%c' but got: '%c'",c, got); }}

#define assert(exp) do { long long e = (long long)(exp); if (! e) {fail(0, "Assertion failed");} } while (0)
