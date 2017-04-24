#pragma once

#include "defines.h"

#include <inttypes.h>
#include <stdio.h>
#include <time.h>
#include <stdbool.h>

typedef enum
{
	UNKNOWN,

	AUDIO,
	RADIO,

	EXTERN,			/* Care */
	OTHER
} ent_create_t;

/* Header for key files */
typedef struct
{
	kid_t kid;                  /* The identifier of the file, probably also the filename */
	len_t start_pos;			/* Where the file starts, if a file got splittet into multiple files or when using std-streams */
	len_t data_len;				/* The length of the entropy data */
	struct tm create_date;      /* UTC Creation date. Always use recent data. */
} key_header_t;

key_header_t* key_header_read(FILE* file);
key_header_t* key_header_create(kid_t kid, len_t start_pos, struct tm* create_date);
void key_header_delete(key_header_t* header);
int key_header_print(key_header_t* header, FILE* out);

typedef struct
{
	key_header_t* head;
	FILE* file;
	char* buffer;
} kkey_t;

kkey_t* key_read(FILE* file);
int key_write(kkey_t* ent, FILE* file, int header_only);
kkey_t* key_create(key_header_t* header, FILE* file);
void key_delete(kkey_t* ent);
int key_print(kkey_t* ent, FILE* out);
