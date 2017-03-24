#include "ent.h"
#include "io.h"
#include "fail.h"

#include <assert.h>
#include <stdlib.h>
#include <sys/stat.h>

int ent_header_write(ent_header_t const* header, FILE* file)
{
	assert(header && file);

	if (fputc('{', file) == EOF
		|| write_uint(header->kid, sizeof(kid_t) << 3, file)
		|| fputc(',', file) == EOF
		|| write_uint(header->start_pos, sizeof(len_t) << 3, file)
		|| fputc(',', file) == EOF
		|| write_uint(header->data_len, sizeof(len_t) << 3, file)
		|| fputc(',', file) == EOF
		|| write_tm(&header->create_date, file)
		|| fputc('}', file) == EOF)
	{
		return fail(0, "File write error");
	}

	return 0;
}

ent_header_t* ent_header_read(FILE* file)
{
	assert(file);

	ent_header_t* ret = (ent_header_t*)malloc(sizeof(ent_header_t));
	if (! ret)
		return fail(0, "Malloc error"), NULL;

	skip(file, '{');
	ret->kid = read_uint(sizeof(kid_t) << 3, file);
	skip(file, ',');
	ret->start_pos = read_uint(sizeof(len_t) << 3, file);
	skip(file, ',');
	ret->data_len = read_uint(sizeof(len_t) << 3, file);
	skip(file, ',');
	ret->create_date = read_tm(file);
	skip(file, '}');

	/* Check expiration date */
	{

	}

	return ret;
}

ent_header_t* ent_header_create(kid_t kid, len_t start_pos, struct tm* create_date)
{
	ent_header_t* ret = (ent_header_t*)malloc(sizeof(ent_header_t));
	if (! ret)
		return fail(0, "Malloc error"), NULL;

	ret->kid = kid;
	ret->start_pos = start_pos;
	ret->data_len = -1;
	if (! create_date)
	{
		time_t ltime;
		struct tm* utc;
		time(&ltime);
		ret->create_date = *gmtime(&ltime);
	}
	else
	{
		ret->create_date = *create_date;
	}

	return ret;
}

void ent_header_delete(ent_header_t* header)
{
	assert(header);

	free(header);
}

int ent_header_print(ent_header_t* header, FILE* out)
{
	assert(header);

	return fprintf(out,
			"kid: %"  PRIx64 "\n\
			start_pos: %" PRIx64 "\n\
			data_len: %" PRIx64 "\n\
			create_date: %s\n", header->kid, header->start_pos, header->data_len, asctime(&header->create_date));
}

ent_t* ent_read(FILE* file)
{
	assert(file);

	ent_header_t* head = ent_header_read(file);
	if (! head)
		return fail(0, "Malloc error"), NULL;

	return ent_create(head, file);
}

ent_t* ent_create(ent_header_t* header, FILE* file)
{
	assert(header && file);

	ent_t* ret = (ent_t*)malloc(sizeof(ent_t));
	if (! ret)
		return fail(0, "Malloc error"), NULL;

	ret->head = header;
	ret->file = file;

	//fprintf(stderr, "Keystream is seekable: %s\n", is_not_seekable(file) ? "no" : "yes");
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

void ent_delete(ent_t* ent)
{
	assert(ent);

	ent_header_delete(ent->head);
	fflush(ent->file);
	if (ent->buffer)
		free(ent->buffer);
	free(ent);
}

int ent_write(ent_t* ent, FILE* file)
{
	assert(ent && file);

	if (ent_header_write(ent->head, file))
	{
		return fail(0, "Error writing key header file");
	}
	if (ent->buffer)
	{
		if (fwrite(ent->buffer, 1, ent->head->data_len, file) != ent->head->data_len)
			return fail(0, "File write error");
	}
	else if (fsplice(ent->file, file, ent->head->data_len) != ent->head->data_len)
	{
		return fail(0, "Error writing key file");
	}

	return 0;
}
