/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2017 by Slavey Karadzhov
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * AtClient.h
 *
 ****/

/**	@defgroup serial AtCommand serial
 *	@brief	Asynchronous AT command client
 *  @{
 */

#pragma once

#include "HardwareSerial.h"
#include "FILO.h"
#include "Timer.h"

#define AT_REPLY_OK "OK"
#ifndef AT_TIMEOUT
#define AT_TIMEOUT 2000
#endif

class AtClient;

typedef Delegate<bool(AtClient& atClient, Stream& source)> AtReceiveCallback;
// ^ If the callback returns true then this means that we have
//     finished successfully processing the command
typedef Delegate<bool(AtClient& atClient, String& reply)> AtCompleteCallback;
// ^ If the callback returns true then this means that we have
//     finished successfully processing the command

typedef struct {
	String text;				   ///< the actual AT command
	String response2;			   ///< alternative successful response
	unsigned timeout;			   ///< timeout in milliseconds
	unsigned retries;			   ///< number of retries before giving up
	bool breakOnError = true;	  ///< stop executing next command if that one has failed
	AtReceiveCallback onReceive;   ///< if set you can process manually all incoming data in a callback
	AtCompleteCallback onComplete; ///< if set then you can process the complete response manually
} AtCommand;

typedef enum { eAtOK = 0, eAtRunning, eAtError } AtState;

/**
 * @brief Class that facilitates the communication with an AT device.
 */
class AtClient
{
public:
	AtClient(HardwareSerial* stream);

	virtual ~AtClient()
	{
	}

	/**
	 * @brief Sends AT command
	 * @param text String The actual AT command text. For example AT+CAMSTOP
	 * @param altResponse String Expected response on success in addition to the default one which is OK
	 * @param timeoutMs uint32_t Time in milliseconds to wait for response
	 * @param retries unsigned Retries on error
	 */
	void send(const String& text, const String& altResponse = nullptr, uint32_t timeoutMs = AT_TIMEOUT,
			  unsigned retries = 0);

	/**
	 * @brief Sends AT command
	 * @param text String The actual AT command text. For example AT+CAMSTOP
	 * @param AtReceiveCallback onReceive
	 * @param timeoutMs uint32_t Time in milliseconds to wait for response
	 * @param retries int Retries on error
	 */
	void send(const String& text, AtReceiveCallback onReceive, uint32_t timeoutMs = AT_TIMEOUT, unsigned retries = 0);

	/**
	 * @brief Sends AT command
	 * @param text String The actual AT command text. For example AT+CAMSTOP
	 * @param AtCompleteCallback onComplete
	 * @param timeoutMs uint32_t Time in milliseconds to wait for response
	 * @param retries int Retries on error
	 */
	void send(const String& text, AtCompleteCallback onComplete, uint32_t timeoutMs = AT_TIMEOUT, unsigned retries = 0);

	// Low Level Functions

	/**
	 * @brief Adds a command to the queue.
	 * 		  If you need all the flexibility then use that command
	 * 		  and manually set your AtCommand arguments.
	 * @param command AtCommand
	 */
	void send(AtCommand command);

	/**
	 * @brief Executes directly (does not queue it) a command
	 * @param command AtCommand
	 */
	void sendDirect(AtCommand command);

	/**
	 * @brief Returns the current state
	 * @return AtState
	 */
	AtState getState()
	{
		return state;
	}

	/*
	 * @brief Repeats the execution of the current command
	 * 		  Useful if the current State is not eAtOK
	 */
	void resend();

	/*
	 * @brief Replaces the current command with the next on in the queue
	 */
	void next();

	AtCommand currentCommand; ///< The current command

protected:
	/**
	 * @brief Processes response data.
	*/
	virtual void processor(Stream& source, char arrivedChar, uint16_t availableCharsCount);

private:
	FIFO<AtCommand, 10> queue;		  ///< Queue for the commands to be executed
	HardwareSerial* stream = nullptr; ///< The main communication stream
	Timer commandTimer;				  ///< timer used for commands with timeout
	AtState state = eAtOK;

	/**
	 * @brief Timeout checker method
	 */
	void ticker();
};

/**
 *  @code   AtClient camera(&Serial);
 *  camera("ATE0\r");
 *  camera("AT+CAMSTOP\r");
 *
 *  @endcode
 */

/** @} */
