#!/bin/sh
#
# Copyright (c) 2007-2016, Cameron Rich
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice,
#   this list of conditions and the following disclaimer.
# * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
# * Neither the name of the axTLS project nor the names of its
#   contributors may be used to endorse or promote products derived
#   from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
# TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

#
# Generate the certificates and keys for testing.
#

PROJECT_NAME="Sming Project"

# Generate the openssl configuration files.
cat > ca_cert.conf << EOF  
[ req ]
distinguished_name     = req_distinguished_name
prompt                 = no

[ req_distinguished_name ]
 O                      = $PROJECT_NAME Dodgy Certificate Authority
EOF

cat > certs.conf << EOF  
[ req ]
distinguished_name     = req_distinguished_name
prompt                 = no

[ req_distinguished_name ]
 O                      = $PROJECT_NAME
 CN                     = 127.0.0.1
EOF

cat > device_cert.conf << EOF  
[ req ]
distinguished_name     = req_distinguished_name
prompt                 = no

[ req_distinguished_name ]
 O                      = $PROJECT_NAME Device Certificate
EOF

# private key generation
openssl genrsa -out ca_key.pem 1024
openssl genrsa -out key_512.pem 512
openssl genrsa -out key_1024.pem 1024
openssl genrsa -out key_1042.pem 1042
openssl genrsa -out key_2048.pem 2048
openssl genrsa -out key_4096.pem 4096
openssl genrsa -out device_key.pem 1024
openssl genrsa -aes128 -passout pass:abcd -out key_aes128.pem 512
openssl genrsa -aes256 -passout pass:abcd -out key_aes256.pem 512


# convert private keys into DER format
openssl rsa -in key_512.pem -out key_512 -outform DER
openssl rsa -in key_1024.pem -out key_1024 -outform DER
openssl rsa -in key_1042.pem -out key_1042 -outform DER
openssl rsa -in key_2048.pem -out key_2048 -outform DER
openssl rsa -in key_4096.pem -out key_4096 -outform DER
openssl rsa -in device_key.pem -out device_key -outform DER

# cert requests
openssl req -out ca_x509.req -key ca_key.pem -new \
            -config ./ca_cert.conf 
openssl req -out x509_512.req -key key_512.pem -new \
            -config ./certs.conf 
openssl req -out x509_1024.req -key key_1024.pem -new \
            -config ./certs.conf 
openssl req -out x509_1042.req -key key_1042.pem -new \
            -config ./certs.conf 
openssl req -out x509_2048.req -key key_2048.pem -new \
            -config ./certs.conf 
openssl req -out x509_4096.req -key key_4096.pem -new \
            -config ./certs.conf 
openssl req -out x509_device.req -key device_key.pem -new \
            -config ./device_cert.conf
openssl req -out x509_aes128.req -key key_aes128.pem \
            -new -config ./certs.conf -passin pass:abcd
openssl req -out x509_aes256.req -key key_aes256.pem \
            -new -config ./certs.conf -passin pass:abcd

# generate the actual certs.
openssl x509 -req -in ca_x509.req -out ca_x509.pem \
            -sha1 -days 5000 -signkey ca_key.pem
openssl x509 -req -in x509_512.req -out x509_512.pem \
            -sha1 -CAcreateserial -days 5000 \
            -CA ca_x509.pem -CAkey ca_key.pem
openssl x509 -req -in x509_1024.req -out x509_1024.pem \
            -sha1 -CAcreateserial -days 5000 \
            -CA ca_x509.pem -CAkey ca_key.pem
openssl x509 -req -in x509_1042.req -out x509_1042.pem \
            -sha1 -CAcreateserial -days 5000 \
            -CA ca_x509.pem -CAkey ca_key.pem
openssl x509 -req -in x509_2048.req -out x509_2048.pem \
            -md5 -CAcreateserial -days 5000 \
            -CA ca_x509.pem -CAkey ca_key.pem
openssl x509 -req -in x509_4096.req -out x509_4096.pem \
            -md5 -CAcreateserial -days 5000 \
            -CA ca_x509.pem -CAkey ca_key.pem
openssl x509 -req -in x509_device.req -out x509_device.pem \
            -sha1 -CAcreateserial -days 5000 \
            -CA x509_512.pem -CAkey key_512.pem
openssl x509 -req -in x509_aes128.req \
            -out x509_aes128.pem \
            -sha1 -CAcreateserial -days 5000 \
            -CA ca_x509.pem -CAkey ca_key.pem
openssl x509 -req -in x509_aes256.req \
            -out x509_aes256.pem \
            -sha1 -CAcreateserial -days 5000 \
            -CA ca_x509.pem -CAkey ca_key.pem

# note: must be root to do this
DATE_NOW=$(date)
if date -s "Jan 1 2025"; then
openssl x509 -req -in x509_512.req -out x509_bad_before.pem \
            -sha1 -CAcreateserial -days 365 \
            -CA ca_x509.pem -CAkey ca_key.pem
date -s "$DATE_NOW"
touch x509_bad_before.pem
fi
openssl x509 -req -in x509_512.req -out x509_bad_after.pem \
            -sha1 -CAcreateserial -days -365 \
            -CA ca_x509.pem -CAkey ca_key.pem

# some cleanup
rm *.req
rm srl
rm *.conf

# need this for the client tests
openssl x509 -in ca_x509.pem -outform DER -out ca_x509.cer 
openssl x509 -in x509_512.pem -outform DER -out x509_512.cer
openssl x509 -in x509_1024.pem -outform DER -out x509_1024.cer
openssl x509 -in x509_1042.pem -outform DER -out x509_1042.cer
openssl x509 -in x509_2048.pem -outform DER -out x509_2048.cer
openssl x509 -in x509_4096.pem -outform DER -out x509_4096.cer
openssl x509 -in x509_device.pem -outform DER -out x509_device.cer

# generate pkcs8 files (use RC4-128 for encryption)
openssl pkcs8 -in key_512.pem -passout pass:abcd -topk8 -v1 PBE-SHA1-RC4-128 -out encrypted_pem.p8
openssl pkcs8 -in key_512.pem -passout pass:abcd -topk8 -outform DER -v1 PBE-SHA1-RC4-128 -out encrypted.p8
openssl pkcs8 -in key_512.pem -nocrypt -topk8 -out unencrypted_pem.p8
openssl pkcs8 -in key_512.pem -nocrypt -topk8 -outform DER -out unencrypted.p8

# generate pkcs12 files (use RC4-128 for encryption)
openssl pkcs12 -export -in x509_1024.pem -inkey key_1024.pem -certfile ca_x509.pem -keypbe PBE-SHA1-RC4-128 -certpbe PBE-SHA1-RC4-128 -name "p12_with_CA" -out withCA.p12 -password pass:abcd
openssl pkcs12 -export -in x509_1024.pem -inkey key_1024.pem -keypbe PBE-SHA1-RC4-128 -certpbe PBE-SHA1-RC4-128 -name "p12_without_CA" -out withoutCA.p12 -password pass:abcd
openssl pkcs12 -export -in x509_1024.pem -inkey key_1024.pem -keypbe PBE-SHA1-RC4-128 -certpbe PBE-SHA1-RC4-128 -out noname.p12 -password pass:abcd

# PEM certificate chain
cat ca_x509.pem >> x509_device.pem

# set default key/cert for use in the server
xxd -i x509_1024.cer | sed -e \
        "s/axTLS_x509_1024_cer/default_certificate/" > $SSL_INCLUDE_DIR/cert.h
xxd -i key_1024 | sed -e \
        "s/axTLS_key_1024/default_private_key/" > $SSL_INCLUDE_DIR/private_key.h
