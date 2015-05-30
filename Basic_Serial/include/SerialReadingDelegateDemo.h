#ifndef INCLUDE_SERIALREADINGDELEGATEDEMO_H_
#define INCLUDE_SERIALREADINGDELEGATEDEMO_H_


class SerialReadingDelegateDemo
{
public:
	void begin()
	{
		Serial.setCallback(StreamDataAvailableDelegate(&SerialReadingDelegateDemo::hwsDelegate,this));
		debugf("hwsDelegateDemo instantiated, waiting for data");
	};

	void hwsDelegate(Stream& stream, char recvChar, unsigned short charCount)
	{
		Serial.print("hwsDelegateDemo Delegate Time = ");
		Serial.print(micros());
		Serial.print(" charCount = ");
		Serial.print(charCount);
		Serial.print(" character = ");
		Serial.println(recvChar);

		numCallback++;

		if (recvChar == 'X') // Toggle useRxBuff
		{
			useRxFlag = !useRxFlag;
			Serial.setCallback(StreamDataAvailableDelegate(&SerialReadingDelegateDemo::hwsDelegate,this),useRxFlag);
		}

		if (charCount >= 10) // Just for example
		{
			while (stream.available())
			{
				char c = stream.read();
				charReceived++;
				Serial.print("Received ");
				Serial.print(charReceived);
				Serial.print(" Characters, this is : ");
				Serial.println(c);
			}
		}
	}

private:
	unsigned charReceived = 0;
	unsigned numCallback = 0;
	bool useRxFlag = true;
};


#endif /* INCLUDE_SERIALREADINGDELEGATEDEMO_H_ */
