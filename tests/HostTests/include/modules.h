// List of test modules to register

// Architecture-specific test modules
#ifdef ARCH_HOST
#define ARCH_TEST_MAP(XX) XX(Hosted)
#else
#define ARCH_TEST_MAP(XX)
#endif

#define TEST_MAP(XX)                                                                                                   \
	XX(Libc)                                                                                                           \
	XX(PreCache)                                                                                                       \
	XX(BitSet)                                                                                                         \
	XX(String)                                                                                                         \
	XX(ArduinoString)                                                                                                  \
	XX(Wiring)                                                                                                         \
	XX(Crypto)                                                                                                         \
	XX(CStringArray)                                                                                                   \
	XX(Stream)                                                                                                         \
	XX(TemplateStream)                                                                                                 \
	XX(Serial)                                                                                                         \
	XX(ObjectMap)                                                                                                      \
	XX(Base64)                                                                                                         \
	XX(DateTime)                                                                                                       \
	XX(Http)                                                                                                           \
	XX(Url)                                                                                                            \
	XX(ArduinoJson5)                                                                                                   \
	XX(ArduinoJson6)                                                                                                   \
	XX(Storage)                                                                                                        \
	XX(Files)                                                                                                          \
	XX(SpiFlash)                                                                                                       \
	XX(Spiffs)                                                                                                         \
	XX(Rational)                                                                                                       \
	XX(Clocks)                                                                                                         \
	XX(Timers)                                                                                                         \
	XX(HttpRequest)                                                                                                    \
	XX(TcpClient)                                                                                                      \
	ARCH_TEST_MAP(XX)
