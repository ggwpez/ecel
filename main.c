#include "message.h"
#include "ent.h"
#include "fail.h"

#define _GNU_SOURCE
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <time.h>

#define BUFF_SIZE 1 << 20

FILE*
open_file(char const* path, char const* mode)
{
	FILE* file = fopen(path, mode);

	if (! file)
	{
		fail(0, "File not found: '%s'\n", path);
		return NULL;
	}
	else
		return file;
}

int
print_usage()
{
	fputs("\
ecel - easy crypto easy life\n\
usage:\n\
	ecel --create-msg --kid=<number> [--pos=<number>] [--raw=<msg_file>]\n\
	ecel --create-key --kid=<number> [--pos=<number>] [--raw=<key_file>]\n\
	ecel --encrypt (--key=<key> | --msg=<message> | --key=<key> --msg=<message>)\n\
\n\
arguments:\n\
	--help          Prints this screen\n\
	--create-msg    Creates a message\n\
	--create-key    Creates a key\n\
	--encrypt       Encrypts a message with a key\n\
	--unalert       Ignores minor errors. NOT ADVISED\n\
	--alert         Disables --unalert\n\
	--raw=<file>    Set a raw input file\n\
	--key=<file>    Set a file created by --create-key\n\
	--msg=<file>    Set a file created by --create-msg\n\
	--pos=<number>  Set the offset in the key\n\
	--kid=<number>  Set the ID of the key\n\
	--output=<file> Redirect stdout to <file>\n\
\n\
Passing - as <file> indicated stdin, which is default behaviour.\n\
All numbers are hexadecimal, even when without \"0x\" or \"h\".\n\
\n\
examples:\n\
	1. Create a key from entropy data:\n\
		ecel --create-key --kid=123 --raw=entropy.file > key.file\n\
	2. Create messge:\n\
		ecel --create-msg --kid=123 --raw=message.file > msg.file\n\
	3. Encrypt:\n\
		ecel --encrypt --key=key.file --msg=msg.file > encrypted_msg.file\n\
	4. Decrypt:\n\
		ecel --encrypt --key=key.file --msg=encrypted_msg.file > normal_msg.file\n\n", stderr);

	return 0;
}

int
main(int argc, char** argv)
{
	int ret = 0;
	assert(sizeof(len_t) >= sizeof(size_t));

	FILE* key_file   = NULL,
		* input_file = NULL,
		* raw_file   = stdin,
		* out_file	 = stdout;

	static int mode = -1;
	kid_t* arg_kid = NULL;
	len_t arg_pos = 0;
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
				{"alert",	   no_argument, &unalert, 0},
				{"unalert",    no_argument, &unalert, 1},

				{"help",	   no_argument, 0, 'h'},
				{"raw",        required_argument, 0, 'r'},
				{"key",        required_argument, 0, 'k'},
				{"msg",        required_argument, 0, 'm'},
				{"pos",        required_argument, 0, 'p'},
				{"kid",        required_argument, 0, 'i'},
				{"output",     required_argument, 0, '0'},

				{0, 0, 0, 0}
			};
			/* getopt_long stores the option index here. */
			int option_index = 0;

			c = getopt_long (argc, argv, "m:k:r:i:p:o:h",
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
						printf (" with arg %s\n", optarg);
				break;

				case 'p':
					arg_pos = strtoull(optarg, NULL, 16);
				break;

				case 'i':
					arg_kid = (kid_t*)malloc(sizeof(kid_t));
					*arg_kid = strtoull(optarg, NULL, 16);
				break;

				case 'r':
					if (! strcmp("-", optarg)) /* - denotes stdin */
						raw_file = stdin;
					else
						raw_file = open_file(optarg, "r");
				break;

				case 'k':
					if (! strcmp("-", optarg))
						key_file = stdin;
					else
						key_file = open_file(optarg, "r");
				break;

				case 'm':
					if (! strcmp("-", optarg))
						input_file = stdin;
					else
						input_file = open_file(optarg, "r");
				break;

				case 'h':
					print_usage();
					fcloseall();
					return 0;
				break;

				case 'o':
					if (! strcmp("-", optarg))
						out_file = stdout;
					else
						out_file = open_file(optarg, "w");
				break;

				case '?': default:
					/* getopt_long already printed an error message. */
					abort();
				break;
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
		else if (! arg_kid)
		{
			return fail(0, "Create-msg needs a --kid=123");
		}

		char* raw_buffer = (char*)malloc(BUFF_SIZE *sizeof(char));
		ssize_t raw_len = 0;
		// TODO only reads as maximum BUFF_SIZE, so 1 MB
		if ((raw_len = fread(raw_buffer, sizeof(char), BUFF_SIZE, raw_file)) <= 0)
			return fail(0, "Raw file read error");

		message_t* msg = message_create(*arg_kid, arg_pos, raw_len, raw_buffer);
		assert(msg);

		message_write(msg, out_file);
	}
	else if (mode == 2)		/* Or better a key? */
	{
		if (! raw_file)
		{
			print_usage();
			return 1;
		}
		else if (! arg_kid)
			return fail(0, "Create-key needs a --kid=<number>");

		char* raw_buffer = (char*)malloc(BUFF_SIZE *sizeof(char));
		ssize_t raw_len = 0;
		// TODO only reads as maximum BUFF_SIZE, so 1 MB
		if ((raw_len = fread(raw_buffer, sizeof(char), BUFF_SIZE, raw_file)) <= 0)
			return fail(0, "Raw file read error");

		ent_header_t* header = ent_header_create(*arg_kid, arg_pos, raw_len, NULL, EXTERN, 0);
		ent_t* key = ent_create(header, raw_buffer);
		assert(header && key);

		ret = ent_write(key, out_file);
	}
	else if (mode == 0)		/* Enctypt */
	{
		if (! key_file && ! input_file)	/* Both streams missing? */
		{
			print_usage();
			return 1;
		}
		else if (key_file == stdin && key_file == input_file)
			return fail(0, "Only one stream can read from stdin simultaniously");
		else if ((bool)key_file ^ (bool)input_file)	/* Only one missing? Take stdin */
		{
			if (! key_file) key_file = stdin;
			if (! input_file) input_file = stdin;
		}

		message_t* msg = message_read(input_file);
		ent_t* key = ent_read(key_file);
		assert(msg && key);

		ret = (message_encrypt_xor(msg, key) || message_write(msg, out_file));
	}
	else
		print_usage();

	if (arg_kid)
		free(arg_kid);
	// Close streams
	{
		if (key_file && key_file != stdin)
			fclose(stdin);
		if (input_file && input_file != stdin)
			fclose(input_file);
		if (raw_file && raw_file != stdin)
			fclose(raw_file);
		if (out_file && out_file !=	stdout)
			fclose(out_file);
	}

	return 0;
}
