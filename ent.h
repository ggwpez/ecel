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

typedef enum
{
    READ,
    WRITE
} ent_rw_t;

/* Header for .ent files */
typedef struct  __attribute__((__packed__))
{
    kid_t kid;                  /* The identifier of the file, probably also the filename */
    len_t start_pos;		/* Where the file starts, if a file got splittet into multiple files or when using std-streams */
    len_t data_len;		/* The length of the entropy data */
    struct tm create_date;      /* UTC Creation date. Always use recent data. */
    ent_create_t entropy_type;	/* Method used to create the data */
    ent_rw_t permission;	/* Read ^ Write file */
} ent_header_t;

ent_header_t* ent_header_read(FILE* file);
ent_header_t* ent_header_create(kid_t kid, len_t start_pos, len_t data_len, struct tm* create_date, ent_create_t entropy_type, ent_rw_t permission);
int ent_header_print(ent_header_t* header);

typedef struct
{
    ent_header_t* head;
    bool is_stream;
    union
    {
	FILE* file;
	char* data;
    };
} ent_t;

ent_t* ent_read(FILE* file);
ent_t* ent_create(ent_header_t* header, FILE* file);
