/*
 * AtClient.cpp
 *
 *  Created on: Feb 23, 2017
 *      Author: slavey
 */

#include "AtClient.h"
#include "Clock.h"

#ifndef debugf
#define debugf(fmt, ...)
#endif

AtClient::AtClient(HardwareSerial* stream) : _stream(stream)
{
	_stream->setCallback(StreamDataReceivedDelegate(&AtClient::processor, this));
}

void AtClient::processor(Stream& source, char arrivedChar, uint16_t availableCharsCount)
{
	if (!currentCommand.text.length())
		return;

	if (_state == eAtError) {
		// discard input at error state
		return;
	}

	if (currentCommand.onReceive) {
		if (currentCommand.onReceive(*this, source))
			next();
		return;
	}

	if (arrivedChar != '\n')
		return;

	_commandTimer.stop();
	debugf("Processing: %d ms, %s", millis(), currentCommand.text.substring(0, 20).c_str());

	char response[availableCharsCount];
	for (int i = 0; i < availableCharsCount; i++) {
		response[i] = _stream->read();
		if (response[i] == '\r' || response[i] == '\n')
			response[i] = '\0';
	}

	debugf("Got response: %s", response);

	String reply(response);
	if (reply.indexOf(AT_REPLY_OK) + reply.indexOf(currentCommand.response2) == -2) {
		// we did not get what we wanted. Check if we should repeat.
		if (--currentCommand.retries > 0) {
			sendDirect(currentCommand);
			return;
		}

		if (currentCommand.breakOnError) {
			_state = eAtError;
			return;
		}
	}

	if (currentCommand.onComplete) {
		if (!currentCommand.onComplete(*this, reply))
			return;
	}

	next();
}

void AtClient::send(const String& text, const String& altResponse, uint32_t timeoutMs, int retries)
{
	AtCommand atCommand;
	atCommand.text = text;
	atCommand.response2 = altResponse;
	atCommand.timeout = timeoutMs;
	atCommand.retries = retries;

	send(atCommand);
}

void AtClient::send(const String& text, AtReceiveCallback onReceive, uint32_t timeoutMs, int retries)
{
	AtCommand atCommand;
	atCommand.text = text;
	atCommand.onReceive = onReceive;
	atCommand.timeout = timeoutMs;
	atCommand.retries = retries;

	send(atCommand);
}

void AtClient::send(const String& text, AtCompleteCallback onComplete, uint32_t timeoutMs, int retries)
{
	AtCommand atCommand;
	atCommand.text = text;
	atCommand.onComplete = onComplete;
	atCommand.timeout = timeoutMs;
	atCommand.retries = retries;

	send(atCommand);
}

// Low Level  Communication Functions

void AtClient::send(AtCommand command)
{
	if (currentCommand.text.length())
		_queue.enqueue(command);
	else
		sendDirect(command);
}

void AtClient::sendDirect(AtCommand command)
{
	_state = eAtRunning;
	_commandTimer.stop();
	currentCommand = command;
	_stream->print(command.text);
	debugf("Sent: timeout: %d, current %d ms, name: %s", currentCommand.timeout, millis(),
		   command.text.substring(0, 20).c_str());
	_commandTimer.initializeMs(currentCommand.timeout, TimerDelegate(&AtClient::ticker, this)).startOnce();
}

// Low Level Queue Functions
void AtClient::resend()
{
	_state = eAtOK;
	if (currentCommand.text.length())
		sendDirect(currentCommand);
	else
		next();
}

void AtClient::next()
{
	if (_state == eAtError) {
		debugf("We are at error state! No next");
		return;
	}

	_state = eAtOK;
	currentCommand.text = nullptr;
	if (_queue.count())
		send(_queue.dequeue());
}

void AtClient::ticker()
{
	debugf("Ticker =================> ");
	if (!currentCommand.text.length()) {
		_commandTimer.stop();
		debugf("Error: Timeout without command?!");
		return;
	}

	currentCommand.retries--;
	debugf("Retries: %d", currentCommand.retries);
	if (currentCommand.retries > 0) {
		_commandTimer.restart();
		sendDirect(currentCommand);
		return;
	}

	_state = eAtError;

	debugf("Timeout: %d ms, %s", millis(), currentCommand.text.c_str());
}
