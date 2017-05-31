#include "message.h"
#include "io.h"
#include "fail.h"
#include "defines.h"

#include <stdlib.h>
#include <ctype.h>

int message_write(message_t const* msg, FILE* file, int header_only)
{
	assert(msg && file);

	if (fputc('[', file) == EOF
		|| write_uint(msg->id, sizeof(kid_t) << 3, file)
		|| fputc(',', file) == EOF
		|| write_uint(msg->start_pos, sizeof(len_t) << 3, file)
		|| fputc(',', file) == EOF
		|| write_uint(msg->len, sizeof(len_t) << 3, file)
		|| fputc(']', file) == EOF)
	{
		return fail(0, "File write error");
	}
	if (! header_only)
	{
		if (msg->buffer)
		{
			if (fwrite(msg->buffer, 1, msg->len, file) != msg->len)
				return fail(0, "File write error");
		}
		else if (fsplice(msg->file, file, msg->len) != msg->len)
			return fail(0, "File write error");
	}

	return 0;
}

/* Message format:
 *
 * [kid_t,start_pos,len_t]
 * DATA
 *
 * */
message_t* message_read(FILE* file)
{
	assert(file);

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

	ret->file = file;
	//fprintf(stderr, "Msgstream is seekable: %s\n", is_not_seekable(file) ? "no" : "yes");

	return ret;
}

message_t* message_create(kid_t id, len_t start_pos, FILE* file)
{
	assert(file);
	message_t* ret = (message_t*)malloc(sizeof(message_t));
	if (! ret)
		return fail(0, "Malloc error"), NULL;

	ret->id = id;
	ret->start_pos = start_pos;

	//fprintf(stderr, "Msgstream is seekable: %s\n", is_not_seekable(file) ? "no" : "yes");
	if (is_not_seekable(file))
	{
		ret->buffer = read_file(file, &ret->len);
	}
	else
	{
		ret->len = flen(file);
		ret->buffer = NULL;
	}

	ret->file = file;

	return ret;
}

void message_delete(message_t* msg)
{
	assert(msg);

	fflush(msg->file);
	if (msg->buffer)
		free(msg->buffer);
	free(msg);
}

int message_encrypt(crypto_ptr_t fptr, message_t* msg, kkey_t* ent, FILE* out, bool strip_header)
{
	assert(msg && ent);

	len_t pos1 = ftello(msg->file),
		  pos2 = ftello(ent->file);

	if (msg->id != ent->head->kid)
		return fail(1, "Missmatching key-ids\nKID of message: 0x%" PRIx64 "\nKID of entropy: 0x%" PRIx64, msg->id, ent->head->kid);
	if (msg->start_pos < ent->head->start_pos)
		return fail(1, "Entropy file is AHEAD of message\nPos of message: 0x%" PRIx64 "\nPos of entropy: 0x%" PRIx64, msg->start_pos, ent->head->start_pos);
	if ((msg->start_pos +msg->len) > (ent->head->start_pos +ent->head->data_len))
		return fail(0, "Insufficient entopy data left:\nmsg->pos: 0x%" PRIx64 " msg->len: 0x%" PRIx64 "\nent->pos: 0x%" PRIx64 " ent->len: 0x%" PRIx64, msg->start_pos, msg->len, ent->head->start_pos, ent->head->data_len);

	if (! strip_header)
		message_write(msg, out, 1);
	for (len_t i = 0; i < msg->len; ++i)
	{
		char c1 = msg->buffer ? msg->buffer[i] : fgetc(msg->file);
		char c2 = ent->buffer ? ent->buffer[i] : fgetc(ent->file);

		if (((c1 == EOF) && ferror(msg->file)) || ((c2 == EOF) && ferror(ent->file)))
			return fail(0, "File_msg read error");
		if ((fputc(fptr(c1, c2, i), out) == EOF) && ferror(out))
			return fail(0, "File_key write error");
	}

	fseeko(msg->file, pos1, SEEK_SET);
	fseeko(ent->file, pos2, SEEK_SET);

	return 0;
}

int message_print(message_t* msg, FILE* out)
{
	assert(msg && out);

	write_uint(msg->id, sizeof(kid_t) << 3, out), fprintf(out, "  "),
	write_uint(msg->start_pos, sizeof(len_t) << 3, out), fprintf(out, "  "),
	write_uint(msg->len, sizeof(len_t) << 3, out), fprintf(out, "  ");
}
