#include "message.h"
#include "ent.h"
#include "io.h"
#include "fail.h"
#include "helper.h"

#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>

int print_usage(void);

/* Trying out a new style, pack all must-free data in global struct
 * and register dtor with atexit */
static struct
{
	int argc;

	/* Moved them up for padding */
	int verbose, strip_msg_head;
	int mode, crypto_mode, get_mode;

	char** argv;
	FILE* key_file,
		* msg_file,
		* raw_file,
		* out_file;

	kid_t* arg_kid;
} state;

enum operation_mode
{
	mENCRYPT,
	mCREATE_MSG,
	mCREATE_KEY,
	mINFO_MSG,
	mINFO_KEY,
	mGET
};

enum get_mode
{
	gMSG_KID,
	gMSG_LEN,
	gMSG_DATE,
	gKEY_KID,
	gKEY_LEN,
	gKEY_DATE
};

static struct option long_options[] =
{
	/* Flags */
	{ "help",	    no_argument, 0, 'h'},
	{ "create-key", no_argument, &state.mode, mCREATE_KEY },
	{ "create-msg", no_argument, &state.mode, mCREATE_MSG },
	{ "info-key",	no_argument, &state.mode, mINFO_KEY },
	{ "info-msg",	no_argument, &state.mode, mINFO_MSG },

	{ "encrypt",    optional_argument, 0, 'e' },
	{ "strip-msg-head", optional_argument, 0, 's' },
	{ "verbosity",  optional_argument, 0, 'v' },

	{ "warning",	required_argument, 0, 'w'},
	{ "get",		required_argument, 0, 'g' },
	{ "raw",        required_argument, 0, 'r' },
	{ "key",        required_argument, 0, 'k' },
	{ "msg",        required_argument, 0, 'm' },
	{ "pos",        required_argument, 0, 'p' },
	{ "kid",        required_argument, 0, 'i' },
	{ "output",     required_argument, 0, 'o' },

	/* NULL descriptor */
	{0, 0, 0, 0}
};

static void state_init(int argc, char** argv);
static void state_cleanup(void);
static void print_call(void);

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
			/* getopt_long stores the option index here. */
			int option_index = 0;
			c = getopt_long(argc, argv, "m:k:r:i:p:o:v::e:hs::g:", long_options, &option_index);

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
				case 'g':
				{
					state.mode = mGET;
					if (! strcmp(optarg, "key_kid"))
						state.get_mode = gKEY_KID;
					else
						fail(0, "Get mode not yet supported. //TODO");
				} break;
				case 'e':
				{
					if (optarg)
						state.crypto_mode = (int)_strtoul(optarg, 16);
				} break;
				case 'v':
				{
					if (optarg)
					{
						int verbosity = (int)_strtoul(optarg, 16);
						if (verbosity == 0)
							unalert = 1;
						else if (verbosity == 1)
							unalert = 0;
						else if (verbosity == 2)
						{
							unalert	= 0;
							state.verbose = 1;
						}
						else
							fail(0, "Unknown value for --verbosity=%s", optarg);
					}
					else
					{
						unalert = 0;
						state.verbose = 2;
					}
				} break;
				case 's':
				{
					if (optarg)
						state.strip_msg_head = (int)_strtoul(optarg, 16);
				} break;
				case 'p':
				{
					arg_pos = (len_t)_strtoull(optarg, 16);

					if (! LEN_VALID(arg_pos))
						fail(0, "Position argument (--pos) is invalid");
				} break;
				case 'w':
				{
					unalert = ! _strtoul(optarg, 16);
				}break;
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
						state.msg_file = stdin;
					else
						state.msg_file = open_file(optarg, "r");
				} break;
				case 'h':
				{
					print_usage();
					return 0;
				}
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
				}
			}
		}
	}

	if (state.verbose)
		print_call();

	if (state.mode == mINFO_MSG)
	{
		if (! state.msg_file)
			state.msg_file = stdin;
		// TODO: check that no other file is set, since it would be ignored

		message_t* msg = message_read(state.msg_file);
		assert(msg);

		message_print(msg, state.out_file);
		fputc('\n', state.out_file);
		message_delete(msg);
	}
	else if (state.mode == mINFO_KEY)
	{
		if (! state.key_file)
			state.key_file = stdin;
		// TODO: check that no other file is set, since it would be ignored

		kkey_t* key = key_read(state.key_file);
		assert(key);

		key_print(key, state.out_file);
		fputc('\n', state.out_file);
		key_delete(key);
	}
	else if (state.mode == mGET)
	{
		if (state.get_mode == gKEY_KID)
		{
			if (! state.key_file)
				state.key_file = stdin;
			// TODO: check that no other file is set, since it would be ignored

			kkey_t* key = key_read(state.key_file);
			assert(key);

			write_uint(key->head->kid, sizeof(kid_t) << 3, state.out_file);
			key_delete(key);
		}
		else
			fail(0, "Get mode not yet supported. TODO");
	}
	else if (state.mode == mCREATE_MSG)		/* Shall we create a message? */
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
	else if (state.mode == mCREATE_KEY)		/* Or better a key? */
	{
		if (! state.raw_file)
			return fail(0, "Create-key needs --raw=<file>");
		else if (! state.arg_kid)
			return fail(0, "Create-key needs --kid=<number>");

		key_header_t* header = key_header_create(*state.arg_kid, arg_pos, NULL);
		kkey_t* key = key_create(header, state.raw_file);
		assert(key);

		ret = key_write(key, state.out_file, 0);
		key_delete(key);
	}
	else if (state.mode == mENCRYPT)		/* Encrypt */
	{
		if (! state.key_file && ! state.msg_file)	/* Both streams missing? */
		{
			return fail(0, "Key and input cant be both NULL in encrypt mode");
		}
		else if (state.key_file == stdin && state.msg_file == stdin)
		{
			return fail(0, "Only one stream can read from stdin simultaniously");
		}
		else if ((bool)state.key_file ^ (bool)state.msg_file)	/* Only one missing? Take stdin */
		{
			if (! state.key_file)
				state.key_file = stdin;
			if (! state.msg_file)
				state.msg_file = stdin;
		}

		message_t* msg = message_read(state.msg_file);
		kkey_t* key = key_read(state.key_file);
		assert(msg && key);

		ret = message_encrypt(get_crypto((crypto_t)state.crypto_mode), msg, key, state.out_file, state.strip_msg_head);

		message_delete(msg);
		key_delete(key);
	}
	else
		return fail(0, "No mode set, see --help");

	return ret;
}

static void state_init(int argc, char** argv)
{
	state.argc = argc;
	state.argv = argv;
	state.key_file = state.msg_file = NULL;
	state.raw_file = stdin;
	state.out_file = stdout;
	state.arg_kid  = NULL;
	state.verbose = state.strip_msg_head = 0;
	state.crypto_mode = XOR;
	state.mode = -1;
}

static void state_cleanup(void)
{
	if (state.key_file && state.key_file != stdin)
		fclose(state.key_file);
	if (state.msg_file && state.msg_file != stdin)
		fclose(state.msg_file);
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
	ecel --create-msg --kid=<number> [--pos=<number>] [--raw=<file>]\n\
	ecel --create-key --kid=<number> [--pos=<number>] [--raw=<file>]\n\
	ecel --encrypt (--key=<key> | --msg=<message> | --key=<key> --msg=<message>)\n\
	ecel --get=key_id --key=<key>\n\
\n\
arguments:\n\
	--help          Prints this screen\n\
	--create-msg    Creates a message\n\
	--create-key    Creates a key\n\
	--encrypt       Encrypts a message with a key\n\
	--alert         Disables --unalert\n\
	--raw=<file>    Set a raw input file\n\
	--key=<file>    Set a file created by --create-key\n\
	--msg=<file>    Set a file created by --create-msg\n\
	--pos=<number>  Set the offset in the key\n\
	--kid=<number>  Set the ID of the key\n\
	--output=<file> Redirect stdout to <file>\n\
	--verbosity[=2] 0 quiet, 1 enable warnings (def), 2 enable debug info\n\
\n\
<key> indicates a <file> which was created by --create-key, same for <message>\n\
Passing - as <file> indicates stdin, which is the default behaviour.\n\
All numbers are hexadecimal even without \"0x\" or \"h\".\n\
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
