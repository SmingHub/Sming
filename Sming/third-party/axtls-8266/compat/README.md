Introduction
============
The name "Lwirax" comes from **LW**ip**R**aw and **AX**tls. 
[LWIP raw](http://lwip.wikia.com/wiki/Raw/TCP) is the asynchronous mode in which the Light Weight IP (LWIP) stack works in popular
microcontrollers like ESP8266. As the name states it is designed to fit in embedded devices with limited resources and provides 
the TCP/IP functionality in your application or device. 

[axTLS](http://axtls.sourceforge.net/) is a light weight TLS(aka SSL) implementation. 

Using axTLS with LWIP raw mode is not straight forward. "Lwirax" tries to ease the burden and provides the "glue" between the two.
It is tested and working with [axTLS for ESP8266](https://github.com/igrr/axtls-8266) but should be easy to use in other
platforms. Lwirax is proven to work with axTLS version 1.4.9 till 2.0.0+. It might work also for other versions.

For a fully working project and integration take a look at [Sming Tasty](https://github.com/slaff/Sming). 
Take a look at [TcpConnection.cpp](https://github.com/slaff/Sming/blob/tasty/Sming/SmingCore/Network/TcpConnection.cpp) for a start.
The [Basic_Ssl](https://github.com/slaff/Sming/tree/tasty/samples/Basic_Ssl) sample can be used to make a test SSL connection with google servers.

Usage
=====

If you are using [LWIP raw tcp mode](http://lwip.wikia.com/wiki/Raw/TCP) and want to add SSL support below are the steps that can help you to achieve this with the help of [axTLS]( http://axtls.sourceforge.net/ ).
		
First you have to include the `lwipr_compat.h` header.

```C
#include "compat/lwipr_compat.h"
```

Then in the code block where you initialize the tcp raw connection you should call `axl_init`.
Take a look at the example below:

```C
lwip_init();

/* 
 * The line below should be added AFTER the lwip_init code
 * AND BEFORE the call to tcp_new()
 * The parameter value 10 specifies how many SSL connections are expected
 */ 
axl_init(10);

// .. some more code
tcp = tcp_new();
tcp_sent(tcp, staticOnSent);
tcp_recv(tcp, staticOnReceive);
tcp_err(tcp, staticOnError);
tcp_poll(tcp, staticOnPoll, 4);
// ... and even more code 
res = tcp_connect(tcp, &addr, port, staticOnConnected);


```

Now we should add in our `staticOnConnected` function code to create new ssl context and ssl object. 
In the example below the `sslObj` and `sslContext` are defined as global

```C
// global definitions
SSL *sslObj = NULL;
SSLCTX* sslContext = NULL;

// and some more code...

err_t staticOnConnected(void *arg, struct tcp_pcb *tcp, err_t err)
{
	int clientfd = -1;
	uint32_t options = 0;

	if (tcp == NULL) {
		/* @TODO: Take care to handle error conditions */
		return -1;
	}

	clientfd = axl_append(tcp);
	if(clientfd == -1) {
		printf("Unable to add LWIP tcp -> clientfd mapping\n");
		return ERR_OK;
	}
	
	printf("Connected: ClientId: %d\n", clientfd);
#ifdef SSL_DEBUG
	options |= SSL_DISPLAY_STATES | SSL_DISPLAY_BYTES;
#endif	
	
	// if you want to verify the server certificate later you can also add the following option
	options |= SSL_SERVER_VERIFY_LATER
	
	sslContext = ssl_ctx_new(SSL_CONNECT_IN_PARTS | options, 1); // !!! SSL_CONNECT_IN_PARTS must be in the flags !!!
	sslObj = ssl_client_new(sslContext, clientfd, NULL, 0);

	return ERR_OK;
}
```


Once we are connected we can send and receive information. For the receiving part we can do the following

```C
err_t staticOnReceive(void *arg, struct tcp_pcb *tcp, struct pbuf *pin, err_t err)
{
	struct pbuf* pout;
	int read_bytes = 0;

	printf("Err: %d\n", err);

	if(tcp == NULL || p == NULL) {
		/* @TODO: Take care to handle error conditions */
		return -1;
	}

	read_bytes = axl_ssl_read(sslObj, tcp, p, &pout);
	if(read_bytes > 0) {
	    // free the SSL pbuf and put the decrypted data in the brand new pout pbuf
		if(p != NULL) {
			pbuf_free(p);
		}
		
		printf("Got decrypted data length: %d", read_bytes);
		
		// put the decrypted data in a brand new pbuf
		p = pout;
	
		// @TODO: Continue to work with the p buf containing the decrypted data 
	}

	return ERR_OK;
}
```

In the receiving part you can also add debug code to display more information about the SSL handshake, once it was successful.


```C
err_t staticOnReceive(void *arg, struct tcp_pcb *tcp, struct pbuf *p, err_t err)
{
	static int show_info = 0;
	const char *common_name = NULL;
	
	// ..
	read_bytes = axl_ssl_read(sslObj, tcp, p, &pout);
	if(read_bytes > 0) {
	    // ...
	}
	
	if(!show_info && ssl_handshake_status(sslObj) == SSL_OK) {
		common_name = ssl_get_cert_dn(sslObj, SSL_X509_CERT_COMMON_NAME);
		if (common_name) {
			printf("Common Name:\t\t\t%s\n", common_name);
		}

		// These two functions below can be found in the axtls examples
		display_session_id(sslObj); 
		display_cipher(sslObj);
		show_info = 1;
	}
	
	return ERR_OK;
}

```


And for the sending part we can use the following code sample as a start

```C
void someSendingfunction() {
	uint8_t *out_buf;
	int out_bytes = 0;
	
	// ... take care to store something in the out_buf
	
	axl_ssl_write(sslObj, out_buf, out_bytes);
}

```

Platforms
=========
If you want to adapt Lwirax for other platforms make sure to add your platform specific changes to the *lwip_platform.h* file.

Good luck and send your success stories at slaff@attachix.com.
