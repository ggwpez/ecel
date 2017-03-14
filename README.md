# ecel - easy crypto easy life
## usage:
	ecel --create-msg --kid=<number> [--pos=<number>] [--raw=<msg_file>]
	ecel --create-key --kid=<number> [--pos=<number>] [--raw=<key_file>]
	ecel --encrypt (--key=<key> | --msg=<message> | --key=<key> --msg=<message>)

## arguments:
	--help          Prints this screen
	--create-msg    Creates a message
	--create-key    Creates a key
	--encrypt       Encrypts a message with a key
	--unalert       Ignores minor errors. NOT ADVISED
	--alert         Disables --unalert
	--raw=<file>    Set a raw input file
	--key=<file>    Set a file created by --create-key
	--msg=<file>    Set a file created by --create-msg
	--pos=<number>  Set the offset in the key
	--kid=<number>  Set the ID of the key
	--output=<file> Redirect stdout to <file>

Passing - as <file> indicated stdin, which is default behaviour.
All numbers are hexadecimal, even when without \"0x\" or \"h\".

## examples:
	### 1. Create a key from entropy data:
		ecel --create-key --kid=123 --raw=entropy.file > key.file
	### 2. Create messge:
		ecel --create-msg --kid=123 --raw=message.file > msg.file
	### 3. Encrypt:
		ecel --encrypt --key=key.file --msg=msg.file > encrypted_msg.file
	### 4. Decrypt:
		ecel --encrypt --key=key.file --msg=encrypted_msg.file > normal_msg.filen
