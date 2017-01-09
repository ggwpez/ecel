#include "message.h"

#include <stdlib.h>
#include <ctype.h>

#define skip(f, c) { char got; if ((got = fgetc(f)) != (c)) { fprintf(stderr, "Syntax error\nAwaited: '%c' but got: '%c'", c, got); }}

int message_write(message_t const* msg, FILE* file)
{
    if (fwrite(msg, sizeof(message_t), 1, file) != 1)
    {
	fputs("File write error", stderr);
	return 1;
    }
    
    return 0;
}

/* Message format:
 * 
 * [kid_t,len_t,encoding_t]
 * DATA
 * 
 * */
message_t* message_read(FILE* file)
{
    message_t* ret = (message_t*)malloc(sizeof(message_t));
    
    skip(file, '[');
    {				
	/* Read kid_t */
	ret->id = 0;

	for (size_t i = 0; i < 16; ++i)
	{
	    char c = fgetc(file);
	    if (c == EOF)
	    {
		fputs("Unawaited EOF", stderr);
		return NULL;
	    }
	    else if (! isxdigit(c))
	    {
		fputs("Currently only works with hex input", stderr);
		return NULL;
	    }
	    else
		ret->id = (ret->id << 4) | ((c -'0') & 15);
	}
    }
    skip(file, ']');

    return ret;
}

message_t* message_create(kid_t id, len_t len, char* data)
{
    message_t* ret = (message_t*)malloc(sizeof(message_t));
    
    ret->id = id;
    ret->len = len;
    ret->data = data;
    
    return ret;
}

int message_encrypt_xor(message_t* msg, ent_t* entropy)
{
    if (msg->len > entropy->head->data_len -entropy->head->start_pos)
    {
	fprintf(stderr, "Keyfile '%" PRIx64 "' insufficient for encrypting message '%" PRIx64 "'\n", entropy->head->kid, msg->id);
	return -1;
    }
    
    fseek(entropy->file, entropy->head->start_pos, SEEK_SET);
    
    for (len_t i = 0; i < msg->len; ++i)
    {
	uint8_t key = fgetc(entropy->file);
	msg->data[i] ^= key;
    }
    
    return 0;
}

int message_print(message_t* msg)
{

}
