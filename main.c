#include "message.h"
#include "ent.h"
#include "io.h"
#include "fail.h"
#include "helper.h"

#define _GNU_SOURCE
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>

int print_usage();

// Trying out a new style, pack all must-free data in global struct
// and register dtor with atexit
static struct
{
	int argc;
	char** argv;
	FILE* key_file,
		* input_file,
		* raw_file,
		* out_file;

	kid_t* arg_kid;
	int verbose;
	int mode, crypto_mode;
} state;

static void state_init(int argc, char** argv);
static void state_cleanup(void);
static void print_call();

int
main(int argc, char** argv)
{
	state_init(argc, argv);
	atexit(state_cleanup);
	int ret = 0;

	len_t arg_pos = 0;
	{
		int c;
		while (1)
		{
			static struct option long_options[] =
			{
				/* These options set a flag. */
				{ "create-key", no_argument, &state.mode, 2 },
				{ "create-msg", no_argument, &state.mode, 1 },
				{ "encrypt",    required_argument, 0, 'e' },

				{ "verbose",    optional_argument, 0, 'v' },

				{ "help",	    no_argument, 0, 'h'},
				{ "raw",        required_argument, 0, 'r' },
				{ "key",        required_argument, 0, 'k' },
				{ "msg",        required_argument, 0, 'm' },
				{ "pos",        required_argument, 0, 'p' },
				{ "kid",        required_argument, 0, 'i' },
				{ "output",     required_argument, 0, 'o' },

				{0, 0, 0, 0}
			};
			/* getopt_long stores the option index here. */
			int option_index = 0;
			c = getopt_long(argc, argv, "m:k:r:i:p:o:v::e:h", long_options, &option_index);

			/* Detect the end of the options. */
			if (c == -1)
				break;

			switch (c)
			{
				case 0:
				{
					/* If this option set a flag, do nothing else now. */
					if (long_options[option_index].flag != 0)
						break;
				} break;
				case 'e':
				{
					state.mode = 0;
					state.crypto_mode = (crypto_t)_strtoul(optarg, 10);
				} break;
				case 'v':
				{
					if (optarg)
						state.verbose = _strtoul(optarg, 10);
					else
						state.verbose = 1;
				} break;
				case 'p':
				{
					arg_pos = _strtoull(optarg, 16);
				} break;
				case 'i':
				{
					state.arg_kid = (kid_t*)malloc(sizeof(kid_t));
					*state.arg_kid = _strtoull(optarg, 16);
				} break;
				case 'r':
				{
					if (! strcmp("-", optarg)) /* - denotes stdin */
						state.raw_file = stdin;
					else
						state.raw_file = open_file(optarg, "r");
				} break;
				case 'k':
				{
					if (! strcmp("-", optarg))
						state.key_file = stdin;
					else
						state.key_file = open_file(optarg, "r");
				} break;
				case 'm':
				{
					if (! strcmp("-", optarg))
						state.input_file = stdin;
					else
						state.input_file = open_file(optarg, "r");
				} break;
				case 'h':
				{
					print_usage();
					return 0;
				} break;
				case 'o':
				{
					if (! strcmp("-", optarg))
						state.out_file = stdout;
					else
						state.out_file = open_file(optarg, "w");
				} break;
				case '?': default:
				{
					/* getopt_long already printed an error message. */
					exit(EXIT_FAILURE);
				} break;
			}
		}
	}

	if (state.verbose)
		print_call();

	if (state.mode == 1)		/* Shall we create a message? */
	{
		if (! state.raw_file)
			return fail(0, "Create-msg needs --raw=<file>");
		else if (! state.arg_kid)
			return fail(0, "Create-msg needs --kid=<number>");

		message_t* msg = message_create(*state.arg_kid, arg_pos, state.raw_file);
		assert(msg);

		message_write(msg, state.out_file, 0);
		message_delete(msg);
	}
	else if (state.mode == 2)		/* Or better a key? */
	{
		if (! state.raw_file)
			return fail(0, "Create-key needs --raw=<file>");
		else if (! state.arg_kid)
			return fail(0, "Create-key needs --kid=<number>");

		ent_header_t* header = ent_header_create(*state.arg_kid, arg_pos, NULL);
		ent_t* key = ent_create(header, state.raw_file);
		assert(key);

		ret = ent_write(key, state.out_file);
		ent_delete(key);
	}
	else if (state.mode == 0)		/* Encrypt */
	{
		if (! state.key_file && ! state.input_file)	/* Both streams missing? */
		{
			return fail(0, "key and input cant be both NULL in encrypt mode");
		}
		else if (state.key_file == stdin && state.key_file == state.input_file)
		{
			return fail(0, "Only one stream can read from stdin simultaniously");
		}
		else if ((bool)state.key_file ^ (bool)state.input_file)	/* Only one missing? Take stdin */
		{
			if (! state.key_file)
				state.key_file = stdin;
			if (! state.input_file)
				state.input_file = stdin;
		}

		message_t* msg = message_read(state.input_file);
		ent_t* key = ent_read(state.key_file);
		assert(msg && key);

		ret = message_merge(get_crypto(state.crypto_mode), msg, key, state.out_file);

		message_delete(msg);
		ent_delete(key);
	}
	else
		return fail(0, "No mode set, see --help");

	return 0;
}

static void state_init(int argc, char** argv)
{
	state.argc = argc;
	state.argv = argv;
	state.key_file = state.input_file = NULL;
	state.raw_file = stdin;
	state.out_file = stdout;
	state.arg_kid  = NULL;
	state.verbose = state.crypto_mode = 0;
	state.mode = -1;
}

static void state_cleanup(void)
{
	if (state.key_file && state.key_file != stdin)
		fclose(state.key_file);
	if (state.input_file && state.input_file != stdin)
		fclose(state.input_file);
	if (state.raw_file && state.raw_file != stdin)
		fclose(state.raw_file);
	if (state.out_file && state.out_file != stdout)
		fclose(state.out_file);
	if (state.arg_kid)
		free(state.arg_kid);
}

void
print_call()
{
	for (int i = 0; i < state.argc; ++i)
		fprintf(stderr, "%s ", state.argv[i]);

	fputc('\n', stderr);
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
	--verbose       Displays debug information\n\
\n\
Passing - as <file> indicates stdin, which is default behaviour.\n\
All numbers are hexadecimal, even when without \"0x\" or \"h\".\n\
\n\
examples:\n\
	1. Create a key from entropy data:\n\
		ecel --create-key --kid=123 --raw=entropy.file --output=key.file\n\
	2. Create messge:\n\
		ecel --create-msg --kid=123 --raw=message.file --output=msg.file\n\
	3. Encrypt:\n\
		ecel --encrypt --key=key.file --msg=msg.file --output=encrypted_msg.file\n\
	4. Decrypt:\n\
		ecel --encrypt --key=key.file --msg=encrypted_msg.file --output=normal_msg.file\n\n", stderr);

	return 0;
}
