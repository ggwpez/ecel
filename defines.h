#pragma once

#include <inttypes.h>

typedef uint64_t kid_t;
typedef uint64_t len_t;
_Static_assert(sizeof(char) == 1, "sizeof(char) != 1");
#define skip(f, c) { char got; if ((got = fgetc(f)) != (c)) { fprintf(stderr, "Syntax error\nAwaited: '%c' but got: '%c'", c, got); }}
