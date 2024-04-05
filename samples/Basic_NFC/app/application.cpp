#include <SmingCore.h>
#include <Libraries/MFRC522/MFRC522.h>

#define SS_PIN 4 // D2

namespace
{
SimpleTimer procTimer;

MFRC522 mfrc522(SS_PIN, SS_PIN);

// List of pins where NFC devices may be connected
const uint8_t ss_pin[]{4, 15};

void scanNfc(uint8_t scanner);

void sayHello()
{
	for(auto pin : ss_pin) {
		mfrc522.setControlPins(pin, pin);
		mfrc522.PCD_Init();
		scanNfc(pin);
	}
}

void scanNfc(uint8_t scannerPin)
{
	if(!mfrc522.PICC_IsNewCardPresent()) {
		Serial << _F("Scanning NFC pin #") << scannerPin << _F(" not present") << endl;
		return;
	}

	// Select one of the cards
	if(!mfrc522.PICC_ReadCardSerial()) {
		Serial << _F("Selecting card #") << scannerPin << _F(" failed...") << endl;
	} else {
		// Show some details of the PICC (that is: the tag/card)
		Serial << _F("Card UID on scanner: ") << scannerPin << endl;
		m_printHex("UID", mfrc522.uid.uidByte, mfrc522.uid.size);
	}

	mfrc522.PICC_HaltA();

	// Stop encryption on PCD
	mfrc522.PCD_StopCrypto1();
	mfrc522.PCD_Init();
}

} // namespace

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);

	procTimer.initializeMs<2000>(sayHello).start();
}
