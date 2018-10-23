#pragma once

#include <inttypes.h>
#include <sys/types.h>

typedef uint64_t kid_t;
typedef __off_t len_t;

_Static_assert(sizeof(__off64_t) == sizeof(off_t), "Error");

#define LEN_INVALID ((len_t)-1)
#define LEN_VALID(l) ((len_t)(l) >= 0)
#define LEN_FMT PRIx64
