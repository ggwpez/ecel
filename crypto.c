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
	}

	fail(0, "Unknown crypto_t given: %i", (int)mode);
	return NULL;
}

char crypto_id_msg(int msg, int key, len_t l)
{
	UNUSED(key && l);
	return msg;
}

char crypto_id_key(int msg, int key, len_t l)
{
	UNUSED(msg && l);
	return key;
}

char crypto_xor(int msg, int key, len_t l)
{
	UNUSED(l);
	return (msg ^ key);
}
