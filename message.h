#pragma once

#include "defines.h"
#include "ent.h"

typedef struct __attribute__((__packed__))
{
    kid_t id;
    len_t len;
    char* data;
} message_t;

int message_write(message_t const* msg, FILE* file);
message_t* message_read(FILE* file);
message_t* message_create(kid_t id, len_t len, char* data);
int message_encrypt_xor(message_t* msg, ent_t* entropy);
