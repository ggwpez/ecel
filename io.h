#pragma once

#include "defines.h"

#include <stdlib.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdio.h>
#include <time.h>

int is_not_seekable(FILE* f);
// Returns the length of the file
ssize_t flen(FILE* f);
// Writes in to out
ssize_t fsplice(FILE* in, FILE* out, ssize_t len);

FILE* open_file(char const* path, char const* mode);
char* read_file(FILE* f, len_t* len);
struct tm read_tm(FILE* file);
int write_tm(struct tm const* time, FILE* file);
uint64_t read_uint(int const bits, FILE* file);
int write_uint(uint64_t const v, int const bits, FILE* file);
