#!/bin/bash

# $1 ID of you own key
# $2 Your keyfile
# $3 Keyfile of your partner
# $4 Offset in your key

spos=$4; awk -W interactive -v pos=$spos -v kid=$1 -v key=$2 '{l=length($0)} $0=system("printf " $0 " | ./ecel --create-msg --pos="pos" --kid="kid" | ./ecel --encrypt=2 --key=" key) "\n" {print} {pos=pos+l; printf "Pos (%u): ", pos > "/dev/stderr"}' | nc 127.0.0.1 1238 | while read -r line; do if [ ${#line} -ge 2 ]; then echo "THEE (" `date +"%T"` "): " `echo $line | ./ecel --encrypt=2 --key=$3 --strip-msg-head`; fi; done

exit 0
