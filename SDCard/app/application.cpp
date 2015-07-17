
#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <Libraries/SDCard/SDCard.h>


void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Allow debug output to serial

	SDCard.begin();

	FIL file;
	FRESULT fRes;
	uint32_t t1, t2, td, byteswritten, i;
	uint8_t sensor_data[1024];

	for(i = 0; i < sizeof(sensor_data); i++) 							//init our data
		sensor_data[i] = '0' + (i%10);									//0123..890123...

	t1 = system_get_time();												//get time at test start

	Serial.print("SDCard test - see code for HW setup\n");
	Serial.print("Write 1K in 1K increment\n");
	fRes = f_open(&file, "file1k.txt", FA_WRITE | FA_CREATE_ALWAYS);	//open file

	if (fRes == FR_OK)
	{
		f_write(&file, sensor_data, sizeof(sensor_data), &byteswritten);//write data to the file

		f_close(&file);													//close the file

		if (byteswritten != 1024)
		{
			Serial.print("Write to file FAIL\n");
		}
	}
	else
	{
		Serial.print("fopen FAIL ");
		Serial.print((unsigned int)fRes);								//print failure error code
	}

	t2 = system_get_time();												//get the time at test end

	Serial.print("\nTest end: ");										//print the write speed
	Serial.print(1000000.0f / (t2-t1));
	Serial.print(" kBps\n");

}
