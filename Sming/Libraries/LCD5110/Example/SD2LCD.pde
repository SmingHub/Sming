#include <LCD5110.h>
#include <SD.h>

File myFile;
char SDDate[30]; // store string from SD card
char inChar=-1;
byte index=0;

void setup(void)
{
  //-----LCD initial begins-----//
  LcdInitialise();
  LcdClear();
  drawBox();
 
  gotoXY(7,1);
  LcdString("Nokia 5110");
  gotoXY(4,2);
  LcdString("Scroll Demo");
  
  Serial.begin(9600);
  Serial.print("Initializing SD card...");
  // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
  // Note that even if it's not used as the CS pin, the hardware SS pin 
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output 
  // or the SD library functions will not work. 
   
  if (!SD.begin(10)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
  
  myFile = SD.open("test.txt");
  if (myFile) {
    Serial.println("test.txt:");
    
    // read from the file until there's nothing else in it:
    while (myFile.available()) {
    	Serial.write(myFile.read());
		if(index < 29) // One less than the size of the array
        {
            inChar = Serial.read(); // Read a character
            SDData[index] = inChar; // Store it
            index++; // Increment where to write next
            SDData[index] = '\0'; // Null terminate the string
        }
    }
    // close the file:
    myFile.close();
  } else {
  	// if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
  
}
 
void loop(void)
{
  gotoXY(4,4);
  Scroll("Scrolling Message from www.elechouse.com");
  delay(200);
  Scroll(String(SDdata));
}