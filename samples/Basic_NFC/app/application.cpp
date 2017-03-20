#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include "SerialReadingDelegateDemo.h"
#include <AppSettings.h>
#include "MFRC522.h"
Timer procTimer;
static Timer nfcScanTimer;
SerialReadingDelegateDemo delegateDemoClass;
int helloCounter = 0;

void ICACHE_FLASH_ATTR scanNfc(byte scanner);
//#define RST_PIN	15 // RST-PIN für RC522 - RFID - SPI - Modul GPIO15
#define SS_PIN	4  	// D2
//#define SS_PIN	15  // D8

//MFRC522 mfrc522(SS_PIN, RST_PIN);	// Create MFRC522 instance
MFRC522 mfrc522(SS_PIN, SS_PIN);	// Create MFRC522 instance
byte ss_pin[]={4,15};

void sayHello()
{
	for(int pinNdx=0;pinNdx<2;pinNdx++){
		byte pin=ss_pin[pinNdx];
		mfrc522.setControlPins(pin,pin);
		mfrc522.PCD_Init();    // Init MFRC522
		scanNfc(pin);
	}


}
//----------------------------------
// Helper routine to dump a byte array as hex values to Serial
void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}
//---------------------------------
void ICACHE_FLASH_ATTR
scanNfc(byte scanner){
static int cnt;
	if ( ! mfrc522.PICC_IsNewCardPresent()) {
		Serial.printf("Scanning nfc Scanner:%d \r\n", scanner);
		return;
	}
	if ( ! mfrc522.PICC_ReadCardSerial()) {	// Select one of the cards
		Serial.println("Selecting card failed...");

	}else{
		  // Show some details of the PICC (that is: the tag/card)
		  debugf("Card UID on scanner:%d:",scanner);
		  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
		  Serial.println();
	 }
		mfrc522.PICC_HaltA();
	 // Stop encryption on PCD
	  mfrc522.PCD_StopCrypto1();
	  mfrc522.PCD_Init();    // Init MFRC522


	//nfcScanTimer.restart();
}




void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default

	procTimer.initializeMs(2000, sayHello).start();

	/// Reading callback example:
	//  * Option 1
	//	Set Serial Callback to global routine:
	//	   Serial.setCallback(onDataCallback);

	// 	* Option 2
	//  Instantiate hwsDelegateDemo which includes Serial Delegate class
	//delegateDemoClass.begin();
	//----- NFC
		//MFRC522 mfrc522(SS_PIN, RST_PIN);
		MFRC522 mfrc522(SS_PIN, SS_PIN);
		SPI.begin();
		mfrc522.PCD_Init();    // Init MFRC522

		//nfcScanTimer.initializeMs(50, scanNfc).startOnce();

}
