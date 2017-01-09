#include "ent.h"

#include <stdlib.h>
#include <sys/stat.h>

int ent_header_write(ent_header_t const* header, FILE* file)
{
    if(fwrite(header, sizeof(ent_header_t), 1, file) != 1)
    {
	fputs("File write error", stderr);
	return 1;
    }
    
    return 0;
}

ent_header_t* ent_header_read(FILE* file)
{
    ent_header_t* ret = (ent_header_t*)malloc(sizeof(ent_header_t));
    
    if (fread(ret, sizeof(ent_header_t), 1, file) != 1)
    {
	fputs("File input Error", stderr);
	return NULL;
    }

    /* Assert the file size */
    {
	struct stat st;
	if (fstat(fileno(file), &st) < 0)
	{
	    fputs("File fstat error", stderr);
	    return NULL;
	}
	if (st.st_size && (st.st_size != sizeof(ent_header_t) +ret->data_len)) /* Check also for stream size, std-streams dont have any */
	{
	    fprintf(stderr, "File corrupt st.st_size: %zu sizeof(ent_header_t): %zu ret->data_len: %zu\n", st.st_size, sizeof(ent_header_t), ret->data_len);
	    return NULL;
	}
    }

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

    return ent_create(head, file);
}   

ent_t* ent_create(ent_header_t* header, FILE* file)
{
    ent_t* ret = (ent_t*)malloc(sizeof(ent_t));

    ret->head = header;
    ret->file = file;
    
    return ret;
}
