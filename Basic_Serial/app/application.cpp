#include <user_config.h>
#include <SmingCore/SmingCore.h>

Timer procTimer;

class hwsDelegateDemo
{
	public :
		hwsDelegateDemo()
		{
			Serial.setCallback(HardwareSerialDelegate(&hwsDelegateDemo::hwsDelegate,this));
			debugf("hwsDelegateDemo instantiated");
		};

	 	 ~hwsDelegateDemo() {};

	 	unsigned charReceived = 0;

	 	void hwsDelegate(unsigned short charCount)
	 	{
	 		Serial.print("hwsDelegateDemo Delegate Time = ");
	 		Serial.print(micros());
	 		Serial.print(" charCount = ");
	 		Serial.println(charCount);

	 		if (charCount >= 10)
	 		{
	 			while (Serial.available())
	 			{
	 				char c = Serial.read();
	 				charReceived++;
	 				Serial.print("Received ");
	 				Serial.print(charReceived);
	 				Serial.print(" Characters, this is : ");
	 				Serial.println(c);
	 			}
	 		}
	 	}

};

void sayHello()
{
	Serial.print("Hello Sming! Let's do smart things.");
	Serial.print(" Time : ");
	Serial.println(micros());
	Serial.println();
}

void hwsCallback(unsigned short charCount)
{
	Serial.print("Serial Callback, charCount =");
	Serial.println(charCount);
}

hwsDelegateDemo *hwsDG;

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default

//  Option 1
//	Set Serial Callback to global routine
//	Serial.setCallback(hwsCallback);

// 	Option 2
//  Instantiate hwsDelegateDemo which includes Serial Delegate
	hwsDG = new hwsDelegateDemo();

	procTimer.initializeMs(2000, sayHello).start();
}
