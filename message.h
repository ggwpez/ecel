#pragma once

#include "defines.h"
#include "ent.h"

typedef struct __attribute__((__packed__))
{
	kid_t id;
	len_t len;
	len_t start_pos;		/* Start position in the according entropy file */
	char* data;
} message_t;

int message_write(message_t const* msg, FILE* file);
message_t* message_read(FILE* file);
message_t* message_create(kid_t id, len_t start_pos, len_t len, char* data);
void message_delete(message_t* msg);
int message_encrypt_xor(message_t* msg, ent_t* entropy);
int message_print(message_t* msg);
