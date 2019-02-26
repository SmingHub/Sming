#ifndef _SERIAL_TRANSMIT_DEMO_H_
#define _SERIAL_TRANSMIT_DEMO_H_

#include <SmingCore.h>
#include <Data/Stream/FlashMemoryStream.h>

class SerialTransmitDemo
{
public:
	SerialTransmitDemo(HardwareSerial& serial, IDataSourceStream* stream) : serial(serial), stream(stream)
	{
		serial.onTransmitComplete(TransmitCompleteDelegate(&SerialTransmitDemo::onTransmitComplete, this));
	}

	virtual ~SerialTransmitDemo()
	{
		// Disconnect callback from serial port
		serial.onTransmitComplete(nullptr);
		delete stream;
	}

	void begin()
	{
		sendDataChunk();
	}

protected:
	// Send a chunk of stream data
	void sendDataChunk();

	void onTransmitComplete(HardwareSerial& serial)
	{
		sendDataChunk();
	}

private:
	HardwareSerial& serial;
	IDataSourceStream* stream = nullptr;
	const size_t chunkSize = 32;
};

#endif /* _SERIAL_TRANSMIT_DEMO_H_ */
