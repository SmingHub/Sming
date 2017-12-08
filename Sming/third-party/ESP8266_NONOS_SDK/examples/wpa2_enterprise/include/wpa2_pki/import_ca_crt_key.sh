#!/bin/bash

CA_FILE=./certs/ca.pem
CRT_FILE=./certs/client.crt
KEY_FILE=./certs/client.key

if [ -f $CA_FILE ];then
	echo "unsigned char ca[] = {" > ./ca.h
	./str2hex $CA_FILE >> ./ca.h
	echo "0x00, };" >> ./ca.h
fi

if [ -f $CRT_FILE ];then
	echo "unsigned char client_cert[] = {" > ./client_crt.h
	./str2hex $CRT_FILE >> ./client_crt.h
	echo "0x00, };" >> ./client_crt.h
fi

if [ -f $KEY_FILE ];then
	echo "unsigned char client_key[] = {" > ./client_key.h
	./str2hex $KEY_FILE >> ./client_key.h
	echo "0x00, };" >> ./client_key.h
fi
