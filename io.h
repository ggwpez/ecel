#pragma once

#include "defines.h"

#include <stdlib.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdio.h>
#include <time.h>

struct tm read_tm(FILE* file);
int write_tm(struct tm const* time, FILE* file);
uint8_t read_uint8_t(FILE* file);
int write_uint8_t(uint8_t v, FILE* file);
uint64_t read_uint64_t(FILE* file);
int write_uint64_t(uint64_t v, FILE* file);
