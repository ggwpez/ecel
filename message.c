#include "message.h"
#include "io.h"
#include "fail.h"

#include <assert.h>
#include <stdlib.h>
#include <ctype.h>

int message_write(message_t const* msg, FILE* file)
{
	if (fputc('[', file) == EOF
	|| write_uint(msg->id, sizeof(kid_t) << 3, file)
	|| fputc(',', file) == EOF
	|| write_uint(msg->start_pos, sizeof(len_t) << 3, file)
	|| fputc(',', file) == EOF
	|| write_uint(msg->len, sizeof(len_t) << 3, file)
	|| fputc(']', file) == EOF
	|| fwrite(msg->data, sizeof(char), msg->len, file) != msg->len)
	{
		return fail(0, "File write error");
	}


	return 0;
}

/* Message format:
 *
 * [kid_t,len_t]
 * DATA
 *
 * */
message_t* message_read(FILE* file)
{
	message_t* ret = (message_t*)malloc(sizeof(message_t));
	if (! ret)
		return fail(0, "Malloc error"), NULL;

	skip(file, '[');
	ret->id = read_uint(sizeof(kid_t) << 3, file);
	skip(file, ',');
	ret->start_pos = read_uint(sizeof(len_t) << 3, file);
	skip(file, ',');
	ret->len = read_uint(sizeof(len_t) << 3, file);
	skip(file, ']');

	ret->data = (char*)malloc(sizeof(char) *ret->len);
	if (! ret->data)
		return fail(0, "Malloc error"), NULL;

	if (fread(ret->data, sizeof(char), ret->len, file) != ret->len)
	{
		return fail(0, "File read error"), NULL;
	}

	return ret;
}

message_t* message_create(kid_t id, len_t start_pos, len_t len, char* data)
{
	assert(len && data);
	message_t* ret = (message_t*)malloc(sizeof(message_t));
	if (! ret)
		return fail(0, "Malloc error"), NULL;

	ret->id = id;
	ret->start_pos = start_pos;
	ret->len = len;
	ret->data = data;

	return ret;
}


int message_encrypt_xor(message_t* msg, ent_t* entropy)
{
	if (msg->id != entropy->head->kid)
		return fail(1, "Missmatching key-ids\nKID of message: 0x%" PRIx64 "\nKID of entropy: 0x%" PRIx64, msg->id, entropy->head->kid);
	if (msg->start_pos < entropy->head->start_pos)
		return fail(1, "Entropy file is AHEAD of message\nPos of message: 0x%" PRIx64 "\nPos of entropy: 0x%" PRIx64, msg->start_pos, entropy->head->start_pos);
	if ((msg->start_pos +msg->len) > (entropy->head->start_pos +entropy->head->data_len))
		return fail(0, "Insufficient entopy data left:\nmsg->pos: 0x%" PRIx64 " msg->len: 0x%" PRIx64 "\nent->pos: 0x%" PRIx64 " ent->len: 0x%" PRIx64, msg->start_pos, msg->len, entropy->head->start_pos, entropy->head->data_len);

	for (len_t i = 0; i < msg->len; ++i)
		msg->data[i] ^= entropy->data[msg->start_pos +i];

	return 0;
}

int message_print(message_t* msg)
{
	return printf("[kid %" PRIx64 ",pos %" PRIx64 ",len %" PRIx64 "]%s\n", msg->id, msg->start_pos, msg->len, msg->data);
}
