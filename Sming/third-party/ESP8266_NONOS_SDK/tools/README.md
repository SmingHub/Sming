## SSL tools
Refer to [ESP8266 Non-OS SDK SSL User Manual](http://espressif.com/en/support/download/documents?keys=&field_type_tid%5B%5D=14)

## certificate generation
you **must** choose one of a,b,c to generate your certificate file
### a) if you does not have any certificate file which issued by trusted CA

we would generate self-signed CA certificate,and generate certificate and private key with self-signed CA.

- **change your CN field in your makefile.sh** to your IP address

run the command as the following:

**makefile.sh:**
```
$./makefile.sh
```
generate all of your cert and private key files.

**rmfile.sh**
```
$./rmfile.sh
```

delete all of your cert and private key files.

the certificate files to flash are under the directory bin/

### b) if you just have CA certificate: ca.crt which issued by trusted CA

we would generate CA bin:esp_ca_cert.bin for one-way authentication

- **[important]**if CA certificate file is not called ca.crt, please rename it to ca.crt
- ensure the CA certificate file is in **PEM** format
- copy your ca.crt to tools/ directory

run the command as the following:

**makefile.sh:**

```
$./makefile.sh
```

the command would generate your CA certificate bin

**rmfile.sh**

```
$./rmfile.sh
```

delete all of your generated files

the certificate file to flash is under the directory bin/

### c) if you have both CA certificate:ca.crt and client certificate:client.crt and client key:client.key which issued by trusted CA

we would generate CA bin:esp_ca_cert.bin and client certificate/private key bin:esp_cert_private_key.bin for two-way authentication

- **[important]**if their filename is not called ca.crt,client.crt,client.key, please rename it to ca.crt,client.crt,client.key
- ensure the ca.crt,client.crt,client.key are in **PEM** format
- copy your ca.crt,client.crt,client.key to tools/ directory

run the command as the following:

**makefile.sh:**

```
$./makefile.sh
```

the command would generate all of your CA certificate bin:esp_ca_cert.bin and client bin:esp_cert_private_key.bin.

**rmfile.sh**

```
$./rmfile.sh
```

delete all of your generated files.

the certificate file to flash is under directory bin/

