#include <SmingCore.h>
#include <Libraries/MFRC522/MFRC522.h>

Timer procTimer;
static Timer nfcScanTimer;
int helloCounter = 0;

void ICACHE_FLASH_ATTR scanNfc(byte scanner);

#define SS_PIN 4 // D2

MFRC522 mfrc522(SS_PIN, SS_PIN); // Create MFRC522 instance
byte ss_pin[] = {4, 15};

void sayHello()
{
	for(int pinNdx = 0; pinNdx < 2; pinNdx++) {
		byte pin = ss_pin[pinNdx];
		mfrc522.setControlPins(pin, pin);
		mfrc522.PCD_Init(); // Init MFRC522
		scanNfc(pin);
	}
}
//---------------------------------
static void ICACHE_FLASH_ATTR dump_byte_array(byte* buffer, byte bufferSize)
{
	String hexOut;
	for(byte i = 0; i < bufferSize; i++) {
		hexOut += String(buffer[i], HEX);
	}
	debugf("%s", hexOut.c_str());
}
//---------------------------------
void ICACHE_FLASH_ATTR scanNfc(byte scanner)
{
	static int cnt;
	if(!mfrc522.PICC_IsNewCardPresent()) {
		debugf("Scanning nfc Scanner:%d \r\n", scanner);
		return;
	}
	if(!mfrc522.PICC_ReadCardSerial()) { // Select one of the cards
		debugf("Selecting card failed...");

	} else {
		// Show some details of the PICC (that is: the tag/card)
		debugf("Card UID on scanner:%d:", scanner);
		dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
		debugf();
	}
	mfrc522.PICC_HaltA();
	// Stop encryption on PCD
	mfrc522.PCD_StopCrypto1();
	mfrc522.PCD_Init(); // Init MFRC522

	//nfcScanTimer.restart();
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default

	procTimer.initializeMs(2000, sayHello).start();

	//----- NFC
	MFRC522 mfrc522(SS_PIN, SS_PIN);
	SPI.begin();
	mfrc522.PCD_Init(); // Init MFRC522

	//nfcScanTimer.initializeMs(50, scanNfc).startOnce();
}
