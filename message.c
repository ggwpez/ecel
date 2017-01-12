#include "message.h"
#include "io.h"

#include <assert.h>
#include <stdlib.h>
#include <ctype.h>

int message_write(message_t const* msg, FILE* file)
{
    if (fputc('[', file) == EOF
	|| write_uint64_t(msg->id, file)//fwrite(&msg->id, sizeof(kid_t), 1, file) != 1
	|| fputc(',', file) == EOF
	|| write_uint64_t(msg->len, file)//fwrite(&msg->len, sizeof(len_t), 1, file) != 1
	|| fputc(']', file) == EOF
	|| fwrite(msg->data, sizeof(char), msg->len, file) != msg->len)
    {
	fputs("File write error", stderr);
	return -1;
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
    
    skip(file, '[');
    ret->id = read_uint64_t(file);
    skip(file, ',');
    ret->len = read_uint64_t(file);
    skip(file, ']');

    ret->data = (char*)malloc(sizeof(char) *ret->len);
    
    if (fread(ret->data, ret->len, 1, file) != 1)
    {
	fputs("File read error", stderr);
	return NULL;
    }
    
    return ret;
}

message_t* message_create(kid_t id, len_t start_pos, len_t len, char* data)
{
    assert(len && data);
    message_t* ret = (message_t*)malloc(sizeof(message_t));
    
    ret->id = id;
    ret->start_pos = start_pos;
    ret->len = len;
    ret->data = data;
    
    return ret;
}

int message_encrypt_xor(message_t* msg, ent_t* entropy)
{
    if (msg->start_pos < entropy->head->start_pos)
    {
	fputs("Entropy file is AHEAD of message", stderr);
	return -1;
    }
    else if ((msg->start_pos +msg->len) > (entropy->head->start_pos +entropy->head->data_len))
    {
	fputs("Insufficient entopy data left", stderr);
	return -1;
    }
    
    for (len_t i = 0; i < msg->len; ++i)
    {
	msg->data[i] ^= entropy->data[msg->start_pos +i];
    }
    
    return 0;
}

int message_print(message_t* msg)
{
    printf("[%" PRIx64 ",%" PRIx64 "]%s\n", msg->id, msg->start_pos, msg->data);
}
