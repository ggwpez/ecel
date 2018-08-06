head -c 1024 /dev/urandom > entropy.file
./ecel --create-key --kid=123 --raw=entropy.file > key.file
echo "hello this is a nice message, kappa" > message.file
./ecel --create-msg --kid=123 --raw=message.file > msg.file
./ecel --encrypt --key=key.file --msg=msg.file > encrypted_msg.file
./ecel --encrypt --key=key.file --msg=encrypted_msg.file > normal_msg.file

diff msg.file normal_msg.file
