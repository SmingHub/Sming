/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * AtClient.cpp
 *
 *  Created on: Feb 23, 2017
 *      Author: slavey
 *
 ****/

#include "AtClient.h"

#include <debug_progmem.h>
#include <Clock.h>

AtClient::AtClient(HardwareSerial& stream) : stream(stream)
{
	stream.onDataReceived(StreamDataReceivedDelegate(&AtClient::processor, this));
}

void AtClient::processor(Stream& source, char arrivedChar, uint16_t availableCharsCount)
{
	if(!currentCommand.text.length()) {
		return;
	}

	if(state == State::Error) {
		// discard input at error state
		return;
	}

	if(currentCommand.onReceive) {
		if(currentCommand.onReceive(*this, source)) {
			next();
		}
		return;
	}

	if(arrivedChar != '\n') {
		return;
	}

	commandTimer.stop();
	debug_d("Processing: %d ms, %s", millis(), currentCommand.text.substring(0, 20).c_str());

	char response[availableCharsCount];
	for(int i = 0; i < availableCharsCount; i++) {
		response[i] = stream.read();
		if(response[i] == '\r' || response[i] == '\n') {
			response[i] = '\0';
		}
	}

	debug_d("Got response: %s", response);

	String reply(response);
	if(reply.indexOf(AT_REPLY_OK) + reply.indexOf(currentCommand.response2) == -2) {
		// we did not get what we wanted. Check if we should repeat.
		if(--currentCommand.retries > 0) {
			sendDirect(currentCommand);
			return;
		}

		if(currentCommand.breakOnError) {
			state = State::Error;
			return;
		}
	}

	if(currentCommand.onComplete) {
		if(!currentCommand.onComplete(*this, reply)) {
			return;
		}
	}

	next();
}

void AtClient::send(const String& text, const String& altResponse, uint32_t timeoutMs, unsigned retries)
{
	Command atCommand;
	atCommand.text = text;
	atCommand.response2 = altResponse;
	atCommand.timeout = timeoutMs;
	atCommand.retries = retries;

	send(atCommand);
}

void AtClient::send(const String& text, ReceiveCallback onReceive, uint32_t timeoutMs, unsigned retries)
{
	Command atCommand;
	atCommand.text = text;
	atCommand.onReceive = onReceive;
	atCommand.timeout = timeoutMs;
	atCommand.retries = retries;

	send(atCommand);
}

void AtClient::send(const String& text, CompleteCallback onComplete, uint32_t timeoutMs, unsigned retries)
{
	Command atCommand;
	atCommand.text = text;
	atCommand.onComplete = onComplete;
	atCommand.timeout = timeoutMs;
	atCommand.retries = retries;

	send(atCommand);
}

// Low Level  Communication Functions

void AtClient::send(Command command)
{
	if(currentCommand.text.length()) {
		queue.enqueue(command);
		return;
	}

	sendDirect(command);
}

void AtClient::sendDirect(Command command)
{
	state = State::Running;
	commandTimer.stop();
	currentCommand = command;
	stream.print(command.text);
	debug_d("Sent: timeout: %d, current %d ms, name: %s", currentCommand.timeout, millis(),
			command.text.substring(0, 20).c_str());
	commandTimer.initializeMs(currentCommand.timeout, TimerDelegate(&AtClient::ticker, this)).startOnce();
}

// Low Level Queue Functions
void AtClient::resend()
{
	state = State::OK;
	if(currentCommand.text.length()) {
		sendDirect(currentCommand);
		return;
	}

	next();
}

void AtClient::next()
{
	if(state == State::Error) {
		debug_e("We are at error state! No next");
		return;
	}

	state = State::OK;
	currentCommand.text = "";
	if(queue.count() > 0) {
		send(queue.dequeue());
	}
}

void AtClient::ticker()
{
	debug_d("Ticker =================> ");
	if(!currentCommand.text.length()) {
		commandTimer.stop();
		debug_e("Error: Timeout without command?!");
		return;
	}

	currentCommand.retries--;
	debug_d("Retries: %d", currentCommand.retries);
	if(currentCommand.retries > 0) {
		commandTimer.restart();
		sendDirect(currentCommand);
		return;
	}

	state = State::Error;

	debug_d("Timeout: %d ms, %s", millis(), currentCommand.text.c_str());
}
