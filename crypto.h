#pragma once

#include "defines.h"

typedef enum
{
	ID_MSG = 0,
	ID_KEY = 1,
	XOR = 2
} crypto_t;

typedef char (*crypto_ptr_t)(char,char,len_t);

crypto_ptr_t get_crypto(crypto_t mode);
char crypto_id_msg(char msg, char key, len_t l);
char crypto_id_key(char msg, char key, len_t l);
char crypto_xor(char msg, char key, len_t l);
