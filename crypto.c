#include "crypto.h"
#include "fail.h"

crypto_ptr_t get_crypto(crypto_t mode)
{
	switch (mode)
	{
		case ID_MSG:
			return &crypto_id_msg;
		case ID_KEY:
			return &crypto_id_key;
		case XOR:
			return &crypto_xor;
		default:
			fail(0, "Unknown crypto_t given: %i", (int)mode);
	}
}

char crypto_id_msg(char msg, char key, len_t l)
{
	return msg;
}

char crypto_id_key(char msg, char key, len_t l)
{
	return key;
}

char crypto_xor(char msg, char key, len_t l)
{
	return (msg ^ key);
}
