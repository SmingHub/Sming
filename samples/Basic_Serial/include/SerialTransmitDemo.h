#ifndef _SERIAL_TRANSMIT_DEMO_H_
#define _SERIAL_TRANSMIT_DEMO_H_

#include <SmingCore.h>

class SerialTransmitDemo
{
public:
	virtual ~SerialTransmitDemo()
	{
		// Disconnect callback from serial port
		if(serial) {
			serial->onTransmitComplete(nullptr);
		}
	}

	void begin(HardwareSerial& serial)
	{
		this->serial = &serial;
		serial.onTransmitComplete(TransmitCompleteDelegate(&SerialTransmitDemo::onTransmitComplete, this));
		debugf("SerialTransmitDemo instantiated, waiting for call to startSending()");
	}

	// Send a chunk of stream data
	virtual void sendDataChunk() = 0;

	virtual void onTransmitComplete(HardwareSerial& serial)
	{
		sendDataChunk();
	}

protected:
	HardwareSerial* serial = nullptr;
};

class SerialStreamTransmitDemo : public SerialTransmitDemo
{
public:
	virtual ~SerialStreamTransmitDemo()
	{
		delete stream;
	}

	virtual void sendDataChunk();

	void startSending(IDataSourceStream* newStream)
	{
		stream = newStream;
		sendDataChunk();
	}

private:
	IDataSourceStream* stream = nullptr;
};

// Demonstrates sending data stored in flash memory via serial port
class SerialFlashMemoryTransmitDemo : public SerialTransmitDemo
{
public:
	// Send a chunk of stream data
	virtual void sendDataChunk();

	void startSending(const FlashString& newData)
	{
		data = &newData;
		sendDataChunk();
	}

private:
	const FlashString* data = nullptr;
	unsigned readPos = 0;
};

#endif /* _SERIAL_TRANSMIT_DEMO_H_ */
