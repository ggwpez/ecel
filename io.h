#pragma once

#include "defines.h"

#include <stdlib.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdio.h>
#include <time.h>

struct tm read_tm(FILE* file);
int write_tm(struct tm const* time, FILE* file);
uint64_t read_uint(int bits, FILE* file);
int write_uint(uint64_t v, int bits, FILE* file);
