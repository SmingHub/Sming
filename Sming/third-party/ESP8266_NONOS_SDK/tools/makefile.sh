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
PROJECT_NAME="TLS Project"
mkdir ca
mkdir bin
TrueCA=-1
if [ -f "ca.crt" ]; then
	echo ca.crt is found, generating esp_ca_cert.bin...
	TrueCA=1
else
	echo ca.crt not exist\! We would generate self-signed CA certificate,and generating esp_ca_cert.bin...
	TrueCA=0

# Generate the openssl configuration files.
cat > ca_cert.conf << EOF  
[ req ]
distinguished_name     = req_distinguished_name
prompt                 = no

[ req_distinguished_name ]
 O                      = $PROJECT_NAME Dodgy Certificate Authority
EOF

	
	openssl genrsa -out ca.key 1024	#you can change number 1024 if encryption bits is required
	openssl req -out ca.req -key ca.key -new -config ./ca_cert.conf

	# generate the actual self-signed ca certs.
	openssl x509 -req -in ca.req -out ca.crt -sha1 -days 5000 -signkey ca.key

	cp ca.key ca/

fi
	# reduce cert size if possible
	openssl x509 -in ca.crt -outform DER -out TLS.ca_x509.cer.bak
	openssl x509 -in TLS.ca_x509.cer.bak -inform DER -out TLS.ca_x509.cer

	cp TLS.ca_x509.cer ca/
	cp make_cacert.py ca/
	cd ca/
	python make_cacert.py
	cd ..
	cp ca/esp_ca_cert.bin bin/
	cp ca.crt ca/

echo esp_ca_cert.bin generated OK\!

if [ $TrueCA -eq 1 ];then
	echo trust CA
	if [ -f "client.crt" ] && [ -f "client.key" ]; then
		echo client.crt \&\& client.key are found, generating esp_cert_private_key.bin 
		mkdir client
		mkdir include
		openssl rsa -in client.key -out TLS.key_1024.bak -outform DER
		openssl x509 -in client.crt -outform DER -out TLS.x509_1024.cer.bak

		# reduce cert and key size if possible
		openssl rsa -in TLS.key_1024.bak -out TLS.key_1024 -inform DER
		openssl x509 -in TLS.x509_1024.cer.bak -inform DER -out TLS.x509_1024.cer

		cp TLS.x509_1024.cer client/
		cp TLS.key_1024 client/
		mv client/TLS.x509_1024.cer client/certificate.cer
		mv client/TLS.key_1024 client/private_key.key_1024
		cp make_cert.py client/
		cd client
	
		python make_cert.py
		rm make_cert.py
		mv esp_cert_private_key.bin ../bin/
		cd ..

		# set default cert for use in the client
		xxd -i  TLS.x509_1024.cer | sed -e \
		        "s/TLS_x509_1024_cer/default_certificate/" > cert.h
		# set default key for use in the server
		xxd -i TLS.key_1024 | sed -e \
		        "s/TLS_key_1024/default_private_key/" > private_key.h
		cp cert.h private_key.h include/
		cp client.crt client.key client/
		
	elif [ -f "server.crt" ] && [ -f "server.key" ]; then
		echo server.crt \&\& server.key are found, generating esp_cert_private_key.bin
		mkdir server
		mkdir include
		openssl rsa -in server.key -out TLS.key_1024.bak -outform DER
		openssl x509 -in server.crt -outform DER -out TLS.x509_1024.cer.bak

		# reduce cert and key size if possible
		openssl rsa -in TLS.key_1024.bak -out TLS.key_1024 -inform DER
		openssl x509 -in TLS.x509_1024.cer.bak -inform DER -out TLS.x509_1024.cer

		cp TLS.x509_1024.cer server/
		cp TLS.key_1024 server/
		mv server/TLS.x509_1024.cer server/certificate.cer
		mv server/TLS.key_1024 server/private_key.key_1024
		cp make_cert.py server/
		cd server
	
		python make_cert.py
		rm make_cert.py
		mv esp_cert_private_key.bin ../bin/
		cd ..

		# set default cert for use in the client
		xxd -i  TLS.x509_1024.cer | sed -e \
		        "s/TLS_x509_1024_cer/default_certificate/" > cert.h
		# set default key for use in the server
		xxd -i TLS.key_1024 | sed -e \
		        "s/TLS_key_1024/default_private_key/" > private_key.h
		cp cert.h private_key.h include/
		cp server.crt server.key server/
	else
		echo we would not generate certificate \for ESP chip when trust CA exist and [client.crt \&\& client.key] or [server.crt \&\& server.key] could not be found.
	fi
elif [ $TrueCA -eq 0 ];then
	echo generate cerificate and private key with self-signed CA
	mkdir server
	mkdir client
	mkdir include

cat > server_cert.conf << EOF  
[ req ]
distinguished_name     = req_distinguished_name
prompt                 = no

[ req_distinguished_name ]
 O                      = $PROJECT_NAME
 CN                     = 192.168.111.100
EOF

cat > client_cert.conf << EOF  
[ req ]
distinguished_name     = req_distinguished_name
prompt                 = no

[ req_distinguished_name ]
 O                      = $PROJECT_NAME Device Certificate
 CN                     = 192.168.111.101
EOF

	openssl genrsa -out server.key 1024
	openssl genrsa -out client.key 1024

	# reduce cert and key size if possible
	openssl rsa -in client.key -out TLS.key_1024.bak -outform DER
	openssl rsa -in TLS.key_1024.bak -out TLS.key_1024 -inform DER

	openssl req -out server.req -key server.key -new -config ./server_cert.conf 
	openssl req -out client.req -key client.key -new -config ./client_cert.conf 

	openssl x509 -req -in server.req -out server.crt -sha1 -CAcreateserial -days 5000 -CA ca.crt -CAkey ca.key
	openssl x509 -req -in client.req -out client.crt -sha1 -CAcreateserial -days 5000 -CA ca.crt -CAkey ca.key
	cp server.crt server.key server/

	# reduce cert and key size if possible
	openssl x509 -in client.crt -outform DER -out TLS.x509_1024.cer.bak
	openssl x509 -in TLS.x509_1024.cer.bak -inform DER -out TLS.x509_1024.cer

	cp TLS.x509_1024.cer client/
	cp TLS.key_1024 client/
	mv client/TLS.x509_1024.cer client/certificate.cer
	mv client/TLS.key_1024 client/private_key.key_1024
	cp make_cert.py client/
	cd client
	
	python make_cert.py
	rm make_cert.py
	mv esp_cert_private_key.bin ../bin/
	cd ..

		# set default cert for use in the client
		xxd -i  TLS.x509_1024.cer | sed -e \
		        "s/TLS_x509_1024_cer/default_certificate/" > cert.h
		# set default key for use in the server
		xxd -i TLS.key_1024 | sed -e \
		        "s/TLS_key_1024/default_private_key/" > private_key.h
		mv cert.h private_key.h include/
	cp client.crt client.key client/
		rm *.crt
		rm *.key

else
	echo error happened\!TrueCA didnot initialize.
fi

#delete intermediate file

rm ca/make_cacert.py ca/esp_ca_cert.bin -rf
rm *.conf -rf
rm *.req -rf
rm *.h -rf
rm *.bak
rm *.srl -rf

find -name \*.cer | xargs rm -f
find -name \*.key_1024 | xargs rm -f

echo esp_ca_cert.bin \&\& esp_cert_private_key.bin was generated under bin\/ directory
