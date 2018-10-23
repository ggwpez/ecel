#include "ent.h"
#include "io.h"
#include "fail.h"

#include "defines.h"
#include <stdlib.h>
#include <sys/stat.h>

int ent_header_write(key_header_t const* header, FILE* file)
{
	assert(header && file);

	if (   fputc('{', file) == EOF
		|| write_uint(header->kid, sizeof(kid_t) << 3, file)
		|| fputc(',', file) == EOF
		|| write_lent(header->start_pos, file)
		|| fputc(',', file) == EOF
		|| write_lent(header->data_len, file)
		|| fputc(',', file) == EOF
		|| write_tm(&header->create_date, file)
		|| fputc('}', file) == EOF  )
	{
		return fail(0, "File write error");
	}

	return 0;
}

key_header_t* key_header_read(FILE* file)
{
	assert(file);

	key_header_t* ret = (key_header_t*)malloc(sizeof(key_header_t));
	if (! ret)
	{
		fail(0, "Malloc error");
		return NULL;
	}

	skip(file, '{');
	ret->kid = read_uint(sizeof(kid_t) << 3, file);
	skip(file, ',');
	ret->start_pos = read_lent(file);
	skip(file, ',');
	ret->data_len = read_lent(file);
	skip(file, ',');
	ret->create_date = read_tm(file);
	skip(file, '}');

	/* Check expiration date */
	{

	}

	return ret;
}

key_header_t* key_header_create(kid_t kid, len_t start_pos, struct tm* create_date)
{
	key_header_t* ret = (key_header_t*)malloc(sizeof(key_header_t));
	if (! ret)
	{
		fail(0, "Malloc error");
		return NULL;
	}

	ret->kid = kid;
	ret->start_pos = start_pos;
	ret->data_len = -1;
	if (! create_date)
	{
		time_t ltime;
	//	struct tm* utc; TODO
		time(&ltime);
		ret->create_date = *gmtime(&ltime);
	}
	else
	{
		ret->create_date = *create_date;
	}

	return ret;
}

void key_header_delete(key_header_t* header)
{
	assert(header);

	free(header);
}

int key_header_print(key_header_t* header, FILE* out)
{
	assert(header);

	return fprintf(out,
			"kid: %"  PRIx64 "\n\
			start_pos: %" PRIx64 "\n\
			data_len: %" PRIx64 "\n\
			create_date: %s\n", header->kid, header->start_pos, header->data_len, asctime(&header->create_date));
}

kkey_t* key_read(FILE* file)
{
	assert(file);

	key_header_t* head = key_header_read(file);
	if (! head)
	{
		fail(0, "Malloc error");
		return NULL;
	}

	return key_create(head, file);
}

kkey_t* key_create(key_header_t* header, FILE* file)
{
	assert(header && file);

	kkey_t* ret = (kkey_t*)malloc(sizeof(kkey_t));
	if (! ret)
	{
		fail(0, "Malloc error");
		return NULL;
	}

	ret->head = header;
	ret->file = file;

	// fprintf(stderr, "Keystream is seekable: %s\n", is_not_seekable(file) ? "no" : "yes");
	// Is it a stream like stdin where we dont know the size?
	// Then we have to read all
	if (header->data_len == -1)
	{
		if (is_not_seekable(file))
		{
			assert(ret->buffer = read_file(file, &ret->head->data_len));
		}
		else
		{
			ret->head->data_len = flen(file);
			ret->buffer = NULL;
		}
	}

	return ret;
}

void key_delete(kkey_t* ent)
{
	assert(ent);

	key_header_delete(ent->head);
	fflush(ent->file);
	if (ent->buffer)
		free(ent->buffer);
	free(ent);
}

int key_write(kkey_t* ent, FILE* file, int header_only)
{
	assert(ent && file);

	if (ent_header_write(ent->head, file))
	{
		return fail(0, "Error writing key file header");
	}
	if (! header_only)
	{
		len_t tmp;
		if (ent->buffer)
		{
			if (fwrite(ent->buffer, 1, (size_t)ent->head->data_len, file) != (size_t)ent->head->data_len)
				return fail(0, "File write error");
		}
		else if ((tmp = fsplice(ent->file, file, ent->head->data_len)) != ent->head->data_len)
		{
			fprintf(stderr, "Tried to write %" LEN_FMT " bytes, but only did %" LEN_FMT "\n", ent->head->data_len, tmp);
			return fail(0, "Error writing key file");
		}
	}

	return 0;
}

int key_print(kkey_t* ent, FILE* out)
{
	assert(ent && out);

	write_uint(ent->head->kid, sizeof(kid_t) << 3, out); fprintf(out, "  ");
	write_lent(ent->head->start_pos, out); fprintf(out, "  ");
	write_lent(ent->head->data_len, out); fprintf(out, "  ");
	return fprintf(out, "%i-%i-%i", ent->head->create_date.tm_year +1900, ent->head->create_date.tm_mon +1, ent->head->create_date.tm_mday);
}
