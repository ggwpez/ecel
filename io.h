#pragma once

#include "defines.h"

#include <stdlib.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdio.h>
#include <time.h>

FILE* open_file(char const* path, char const* mode);
struct tm read_tm(FILE* file);
int write_tm(struct tm const* time, FILE* file);
uint64_t read_uint(int const bits, FILE* file);
int write_uint(uint64_t const v, int const bits, FILE* file);
