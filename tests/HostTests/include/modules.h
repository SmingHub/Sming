// List of test modules to register

#ifdef DISABLE_NETWORK
#define XX_NET(test)
#else
#define XX_NET(test) XX(test)
#endif

// Architecture-specific test modules
#ifdef ARCH_HOST
#define ARCH_TEST_MAP(XX)                                                                                              \
	XX_NET(Hosted)                                                                                                     \
	XX_NET(HttpRequest)                                                                                                \
	XX_NET(TcpClient)
#else
#define ARCH_TEST_MAP(XX)
#endif

#define TEST_MAP(XX)                                                                                                   \
	XX(System)                                                                                                         \
	XX(SpiFlash)                                                                                                       \
	XX(Libc)                                                                                                           \
	XX(PreCache)                                                                                                       \
	XX(BitSet)                                                                                                         \
	XX(String)                                                                                                         \
	XX(ArduinoString)                                                                                                  \
	XX(Wiring)                                                                                                         \
	XX_NET(Crypto)                                                                                                     \
	XX(CStringArray)                                                                                                   \
	XX(Stream)                                                                                                         \
	XX(TemplateStream)                                                                                                 \
	XX(Serial)                                                                                                         \
	XX(ObjectMap)                                                                                                      \
	XX_NET(Base64)                                                                                                     \
	XX(DateTime)                                                                                                       \
	XX(Uuid)                                                                                                           \
	XX_NET(Http)                                                                                                       \
	XX_NET(Url)                                                                                                        \
	XX(ArduinoJson5)                                                                                                   \
	XX(ArduinoJson6)                                                                                                   \
	XX(Storage)                                                                                                        \
	XX(Files)                                                                                                          \
	XX(Spiffs)                                                                                                         \
	XX(Rational)                                                                                                       \
	XX(Clocks)                                                                                                         \
	XX(Timers)                                                                                                         \
	ARCH_TEST_MAP(XX)
