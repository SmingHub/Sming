
#include "Context.h"
#include "KeyCertPair.h"

namespace Ssl
{
struct Session {
	Context* context = nullptr;
	Connection* connection = nullptr;
	Extension* extension = nullptr;
	bool connected = false;
	KeyCertPair keyCert;
	bool freeKeyCertAfterHandshake = false;
	SessionId* sessionId = nullptr;
	uint32_t options = 0;
	// For TCP server
	int cacheSize = 50;

	~Session()
	{
		close();

		delete sessionId;
		sessionId = nullptr;
	}

	/**
	 * @brief Called by server to prepare for listening
	 */
	bool listen(tcp_pcb* tcp);

	/**
	 * @brief Handle connection event
	 * @retval err_t
	 */
	err_t onConnected(tcp_pcb* tcp);

	void close();

	/**
	 * @brief Handle received buffer
	 * @param tcp
	 * @param p Received encrypted data, on success contains decrypted packet
	 * @retval int Number of bytes received, or error code
	 */
	int onReceive(tcp_pcb* tcp, pbuf*& p);

	int write(tcp_pcb* tcp, const uint8_t* data, size_t len);
};

}; // namespace Ssl
