#include "ent.h"
#include "io.h"

#include <stdlib.h>
#include <sys/stat.h>

int ent_header_write(ent_header_t const* header, FILE* file)
{
    if (fputc('{', file) == EOF
	|| write_uint(header->kid, sizeof(kid_t) << 3, file)
	|| fputc(',', file) == EOF
	|| write_uint(header->start_pos, sizeof(len_t) << 3, file)
	|| fputc(',', file) == EOF
	|| write_uint(header->data_len, sizeof(len_t) << 3, file)
	|| fputc(',', file) == EOF
	|| write_tm(&header->create_date, file)
	|| fputc(',', file) == EOF
	|| write_uint(header->entropy_type, 4, file)
	|| fputc(',', file) == EOF
	|| write_uint(header->permission, 4, file)
	|| fputc('}', file) == EOF)
    {
	fputs("File write error", stderr);
	return 1;
    }
    
    return 0;
}

ent_header_t* ent_header_read(FILE* file)
{
    ent_header_t* ret = (ent_header_t*)malloc(sizeof(ent_header_t));
    
    skip(file, '{');
    ret->kid = read_uint(sizeof(kid_t) << 3, file);
    skip(file, ',');
    ret->start_pos = read_uint(sizeof(len_t) << 3, file);
    skip(file, ',');
    ret->data_len = read_uint(sizeof(len_t) << 3, file);
    skip(file, ',');
    ret->create_date = read_tm(file);
    skip(file, ',');
    ret->entropy_type = read_uint(4, file);
    skip(file, ',');
    ret->permission = read_uint(4, file);
    skip(file, '}');

    /* Check expiration date */
    {
	
    }
    
    return ret;
}

ent_header_t* ent_header_create(kid_t kid, len_t start_pos, len_t data_len, struct tm* create_date, ent_create_t entropy_type, ent_rw_t permission)
{
    ent_header_t* ret = (ent_header_t*)malloc(sizeof(ent_header_t));
    
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
    ret->entropy_type = entropy_type;
    ret->permission = permission;
    
    return ret;
}

int ent_header_print(ent_header_t* header)
{
        return printf(
"kid: %"  PRIx64 "\n\
start_pos: %" PRIx64 "\n\
data_len: %" PRIx64 "\n\
create_date: %s\
entropy_type: %i\n\
permission: %i\n", header->kid, header->start_pos, header->data_len, asctime(&header->create_date), header->entropy_type, header->permission);
}

ent_t* ent_read(FILE* file)
{
    ent_header_t* head = ent_header_read(file);

    char* buffer = (char*)malloc(head->data_len);
    if (fread(buffer, sizeof(char), head->data_len, file) != head->data_len)
    {
	fputs("File input Error", stderr);
	return NULL;
    }

    return ent_create(head, buffer);
}   

ent_t* ent_create(ent_header_t* header, char* data)
{
    ent_t* ret = (ent_t*)malloc(sizeof(ent_t));

    ret->head = header;
    ret->data = data;
    
    return ret;
}

int ent_write(ent_t* ent, FILE* file)
{
    if (ent_header_write(ent->head, file)
	|| fwrite(ent->data, sizeof(char), ent->head->data_len, file) != ent->head->data_len)
    {
	fputs("Error writing key file", stderr);
	return -1;
    }
    
    return 0;
}

int ent_print(ent_t* ent)
{    
    return ent_header_print(ent->head) +printf("%.*s", (int)ent->head->data_len, ent->data);
}
