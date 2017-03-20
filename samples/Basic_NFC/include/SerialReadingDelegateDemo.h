#ifndef INCLUDE_SERIALREADINGDELEGATEDEMO_H_
#define INCLUDE_SERIALREADINGDELEGATEDEMO_H_

//*** Example of global callback routine
void onDataCallback(Stream& stream, char arrivedChar, unsigned short availableCharsCount)
{
}

//*** Example of class callback processing
class SerialReadingDelegateDemo
{
public:
	void begin()
	{
		Serial.setCallback(StreamDataReceivedDelegate(&SerialReadingDelegateDemo::onData, this));
		debugf("hwsDelegateDemo instantiated, waiting for data");
	};

	void onData(Stream& stream, char arrivedChar, unsigned short availableCharsCount)
	{
		Serial.print("Class Delegate Demo Time = ");
		Serial.print(micros());
		Serial.print(" char = 0x");
		Serial.print(String(arrivedChar, HEX)); // char hex code
		Serial.print(" available = ");
		Serial.println(availableCharsCount);

		numCallback++;

		if (arrivedChar == '\n') // Lets show data!
		{
			Serial.println("<New line received>");
			while (stream.available())
			{
				char cur = stream.read();
				charReceived++;
				Serial.print(cur);
			}
			Serial.println();
		}
	}

private:
	unsigned charReceived = 0;
	unsigned numCallback = 0;
	bool useRxFlag = true;
};


#endif /* INCLUDE_SERIALREADINGDELEGATEDEMO_H_ */
