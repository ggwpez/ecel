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

ent_header_t* ent_header_create(kid_t kid, len_t start_pos, len_t data_len, struct tm* create_date)
{
	ent_header_t* ret = (ent_header_t*)malloc(sizeof(ent_header_t));
	if (! ret)
		return fail(0, "Malloc error"), NULL;

	ret->kid = kid;
	ret->start_pos = start_pos;
	ret->data_len = data_len;
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

int ent_header_print(ent_header_t* header)
{
	assert(header);

	return printf(
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

	char* buffer = (char*)malloc(head->data_len);
	if (! buffer)
		return fail(0, "Malloc error"), NULL;

	if (fread(buffer, sizeof(char), head->data_len, file) != head->data_len)
		return fail(0, "File input Error"), NULL;

	return ent_create(head, buffer);
}

ent_t* ent_create(ent_header_t* header, char* data)
{
	assert(header && data);

	ent_t* ret = (ent_t*)malloc(sizeof(ent_t));
	if (! ret)
		return fail(0, "Malloc error"), NULL;

	ret->head = header;
	ret->data = data;

	return ret;
}

void ent_delete(ent_t* ent)
{
	assert(ent);

	ent_header_delete(ent->head);
	free(ent->data);
	free(ent);
}

int ent_write(ent_t* ent, FILE* file)
{
	assert(ent && file);

	if (ent_header_write(ent->head, file)
		|| fwrite(ent->data, sizeof(char), ent->head->data_len, file) != ent->head->data_len)
	{
		return fail(0, "Error writing key file");
	}

	return 0;
}

int ent_print(ent_t* ent)
{
	assert(ent);

	return ent_header_print(ent->head) +printf("%.*s", (unsigned)ent->head->data_len, ent->data);
}
