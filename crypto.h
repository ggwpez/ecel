#pragma once

#include "defines.h"

typedef enum
{
	ID_MSG = 0,
	ID_KEY = 1,
	XOR = 2
} crypto_t;

typedef int (*crypto_ptr_t)(int,int,len_t);

crypto_ptr_t get_crypto(crypto_t mode);
int crypto_id_msg(int msg, int key, len_t l);
int crypto_id_key(int msg, int key, len_t l);
int crypto_xor(int msg, int key, len_t l);
