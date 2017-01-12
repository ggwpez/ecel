#include "message.h"
#include "ent.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <time.h>

#define BUFF_SIZE 1 << 20

FILE* open_file(char const* path)
{
    FILE* file = fopen(path, "r");
    
    if (! file)
    {
	fprintf(stderr, "File not found: '%s'\n", path);
	exit(-2);
	return NULL;
    }
    else
	return file;
}

int
print_usage()
{
    fputs("Usage:\n\tecel -kkey -iinput\n", stderr);
    
    return 0;
}

int
main(int argc, char** argv)
{
    assert(sizeof(len_t) >= sizeof(size_t));
    
    FILE* key_file   = NULL,
	* input_file = NULL,
	* raw_file   = stdin;

    
    static int mode = -1;
    kid_t* arg_kid = NULL;
    len_t* arg_pos = NULL;
    {
	int c;
	while (1)
	{
	    static struct option long_options[] =
		{
		    /* These options set a flag. */
		    {"create-key", no_argument, &mode, 2},
		    {"create-msg", no_argument, &mode, 1},
		    {"encrypt",    no_argument, &mode, 0},
		
		    {"raw",        required_argument, 0, 'r'},
		    {"key",        required_argument, 0, 'k'},
		    {"msg",        required_argument, 0, 'm'},
		    {"pos",        required_argument, 0, 'p'},
		    {"kid",        required_argument, 0, 'i'},
//		{"start-pos",  required_argument, 0, 'p'},
		    {0, 0, 0, 0}
		};
	    /* getopt_long stores the option index here. */
	    int option_index = 0;

	    c = getopt_long (argc, argv, "m:k:r:i:p:",
			     long_options, &option_index);

	    /* Detect the end of the options. */
	    if (c == -1)
		break;

	    switch (c)
	    {
	    case 0:
		/* If this option set a flag, do nothing else now. */
		if (long_options[option_index].flag != 0)
		    break;
		printf ("option %s", long_options[option_index].name);
		if (optarg)
		    printf (" with arg %s", optarg);
		printf ("\n");
		break;

	    case 'p':
		arg_pos = (len_t*)malloc(sizeof(len_t));
		*arg_pos = strtoull(optarg, NULL, 16);
		break;
		
	    case 'i':
		arg_kid = (kid_t*)malloc(sizeof(kid_t));
		*arg_kid = strtoull(optarg, NULL, 16);
		break;
	    
	    case 'r':
		if (! strcmp("-", optarg)) /* - denotes stdin */
		    raw_file = stdin;
		else
		    raw_file = open_file(optarg);
		break;
	    
	    case 'k':
		if (! strcmp("-", optarg))
		    key_file = stdin;
		else
		    key_file = open_file(optarg);
		break;

	    case 'm':
		if (! strcmp("-", optarg))
		    input_file = stdin;
		else
		    input_file = open_file(optarg);
		break;

	    case '?':
		/* getopt_long already printed an error message. */
		break;

	    default:
		abort();
	    }
	}
    }

    if (mode == 1)		/* Shall we create a message? */
    {
	if (! raw_file)
	{
	    print_usage();
	    return 1;
	}
	else if (! arg_kid || ! arg_pos)
	{
	    fputs("Create-msg needs a --kid=123 and --pos=123", stderr);
	    return 1;
	}

	char* raw_buffer = (char*)malloc(BUFF_SIZE *sizeof(char));
	ssize_t raw_len;
	if ((raw_len = fread(raw_buffer, sizeof(char), BUFF_SIZE, raw_file)) < 0)
	{
	    fputs("Raw file read error", stderr);
	    return 2;
	}

	message_t* msg = message_create(*arg_kid, *arg_pos, raw_len, raw_buffer);
	message_write(msg, stdout);
    }
    else if (mode == 2)		/* Or better a key? */
    {
	if (! raw_file)
	{
	    print_usage();
	    return 1;
	}
	else if (! arg_kid || ! arg_pos)
	{
	    fputs("Create-key needs a --kid=123 and --pos=123", stderr);
	    return 1;
	}
	
	char* raw_buffer = (char*)malloc(BUFF_SIZE *sizeof(char));
	ssize_t raw_len;
	if ((raw_len = fread(raw_buffer, sizeof(char), BUFF_SIZE, raw_file)) < 0)
	{
	    fputs("Raw file read error", stderr);
	    return 2;
	}

	ent_header_t* header = ent_header_create(*arg_kid, *arg_pos, raw_len, NULL, EXTERN, 0);
	ent_t* key = ent_create(header, raw_buffer);
	ent_write(key, stdout);
    }
    else if (mode == 0)
    {
    	if (! key_file && ! input_file)	/* Both streams missing? */
    	{
    	    print_usage();
    	    return 1;
	} else if (key_file == stdin && key_file == input_file)
	{
	    fputs("Only one stream can read from stdin simultaniously", stderr);
	    return 1;
    	}
	else if ((bool)key_file ^ (bool)input_file)	/* Only one missing, take stdin */
    	{
    	    if (! key_file) key_file = stdin;
    	    if (! input_file) input_file = stdin;
    	}

    	message_t* msg = message_read(input_file);
    	ent_t* key = ent_read(key_file);

	message_print(msg);
	puts("\n");
	ent_print(key);
//  	message_encrypt_xor(msg, key);
//	message_write(msg, stdout);
    }
    else
	print_usage();

    if (arg_kid) free(arg_kid);
    if (arg_pos) free(arg_pos);
    
    return 0;
}
