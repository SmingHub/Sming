
#include "Context.h"
#include "KeyCertPair.h"

namespace Ssl
{
struct Session {
	Context* context = nullptr;
	Connection* connection = nullptr;
	Extension extension;
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

	/**
	 * @brief End the session
	 */
	void close();

	/**
	 * @brief Process received encrypted data
	 * @param encrypted The input data, will be freed before returning
	 * @param decrypted On success, the decrypted data
	 * @retval int Number of bytes received, or error code
	 */
	int read(pbuf* encrypted, pbuf*& decrypted);

	/**
	 * @brief Write data to SSL connection
	 * @param data
	 * @param length
	 * @retval int Quantity of bytes actually written
	 */
	int write(const uint8_t* data, size_t length)
	{
		if(connection == nullptr) {
			return ERR_CONN;
		}

		return connection->write(data, length);
	}
};

}; // namespace Ssl
