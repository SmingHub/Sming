#pragma once

#include <Network/Http/HttpServerConnection.h>
#include <Data/Stream/FileStream.h>
#include <Timer.h>

/*
 * It may not always be possible to fulfil an incoming HTTP request immediately.
 * This class provides a simple demonstration of how to defer a file request using a timer.
 * See sendFile() for further details.
 *
 * A more realistic example might be if we need to communicate with a slow Serial device, which
 * might take several seconds. This can be handled asychronously without affecting other system
 * operations.
 */
class DelayStream : public FileStream
{
public:
	DelayStream(const String& filename, HttpServerConnection& connection) : filename(filename), connection(connection)
	{
		debug_i("DelayStream(\"%s\") %p", filename.c_str(), this);
		timer.initializeMs<500>([this]() { sendFile(); });
		timer.startOnce();
	}

	~DelayStream()
	{
		debug_i("~DelayStream() %p", this);
	}

	bool isValid() const override
	{
		return true;
	}

	int available() override
	{
		return ready ? FileStream::available() : 0;
	}

	bool isFinished() override
	{
		return ready && FileStream::isFinished();
	}

private:
	void sendFile();

	String filename;
	HttpServerConnection& connection;
	Timer timer;
	bool ready{false};
};
