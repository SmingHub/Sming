# Compilation
In Sming the SSL support is not enabled by default.

In order to enable it you should first recompile SmingFramework with *ENABLE_SSL=1* directive.
This can be done using the following commands:

```bash
cd <path-to-sming>/Sming
make clean
make ENABLE_SSL=1
```

Once you have enabled the SSL support in SmingFramework you can go forward and compile
your application with the same directive. 
For example the Basic_Ssl project should be compiled with

```bash
cd <path-to-sming>/samples/Basic_Ssl
make clean
make ENABLE_SSL=1
```

Now you can flash your application to your ESP8266 device.

# Debug Information
If you want to see more debug information during compile type you should 
add the directive *SSL_DEBUG=1*. A recompilation of SmingFramework with SSL support
and SSL dubug information can be done with the following commands:


```bash
cd <path-to-sming>/Sming
make clean
make ENABLE_SSL=1 SSL_DEBUG=1
```

# Slow SSL negotiation
The initial SSL negotiation is CPU intensive. By default SmingFramework switches the CPU
frequency from 80 to 160 MHz. After the negotiation the CPU is switched back to 80 MHz.

If your device is running on battery this can drain the battery much faster. If you do not
want the switch from 80 to 160 MHz to happen then make sure to recompile SmingFramework with
*SSL_SLOW_CONNECT* directive.
