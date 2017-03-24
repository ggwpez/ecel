#pragma once

#include "defines.h"
#include "crypto.h"
#include "ent.h"

typedef struct __attribute__((__packed__))
{
	kid_t id;
	len_t len;
	len_t start_pos;		/* Start position in the according entropy file */
	FILE* file;
	char* buffer;
} message_t;

int message_write(message_t const* msg, FILE* file, int header_only);
message_t* message_read(FILE* file);
message_t* message_create(kid_t id, len_t start_pos, FILE* file);
void message_delete(message_t* msg);
int message_merge(crypto_ptr_t fptr, message_t* msg, ent_t* ent, FILE* out);
int message_print(message_t* msg, FILE* out);
