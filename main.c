#include "ent.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

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
    FILE* key   = NULL,
	* input = NULL;
    {
	char c;
	while ((c = getopt (argc, argv, "k:d:rt")) != -1)
	{
	    switch (c)
	    {
	    case 'k':
		key = open_file(optarg);
		break;
		
	    case 'i':
		input = open_file(optarg);
		break;

	    case 't':
	    {
		 srand(time(NULL));
		 
		 ent_header_t* header = ent_header_create((kid_t)(rand()) << 32 | rand(), 0, 0, NULL, EXTERN, WRITE);
		 fwrite(header, sizeof(ent_header_t), 1, stdout);
		 free(header);
		 return 0;
	    } break;

	    case 'r':
	    {
		ent_header_t* header = ent_header_read(stdin);
		ent_header_print(header);
		free(header);
		return 0;
	    } break;
		
	    case '?':
		if (key) fclose(key);
		if (input) fclose(input);
	    
		return print_usage();
	    }
	}
    }
    if (! key && ! input)	/* Both streams missing? */
    {
	print_usage();
	return 1;
    }
    else if ((bool)key ^ (bool)input)	/* Only one missing, take stdin */
    {
	if (! key) key = stdin;
	if (! input) input = stdin;
    }
    
    return 0;
}
