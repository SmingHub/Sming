#include <user_config.h>
#include <SmingCore/SmingCore.h>

#include <Adafruit_NeoPixel/Adafruit_NeoPixel.h>


#ifndef WIFI_SSID
	#define WIFI_SSID "XXX" 			// Put you SSID and Password here
	#define WIFI_PWD "XXX"
#endif

// Which pin on the Esp8266 is connected to the NeoPixels?
#define PIN            15

// How many NeoPixels are attached to the Esp8266?
#define NUMPIXELS      16

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void StartDemo(void);

Timer StripDemoTimer;
Timer ColorWipeTimer;
Timer TheaterChaseTimer;

int StripDemoType = 0;
int StripColor =0;
int StripNo=0;
int ChaseCycle=0;
int TheaterChaseQ=0;


//
// Fill the dots one after the other with a color
//

void ColorWipe() {

	if (StripNo < strip.numPixels())
	{
		strip.setPixelColor(StripNo, StripColor);
		strip.show();
		StripNo++;
	}
	else
	{
		StripDemoType++;         // next demo type
		ColorWipeTimer.stop();   // stop this demo timer
		StripDemoTimer.initializeMs(2000, StartDemo).start(true);  // start next demo after 2 seconds
	}

}


//
//Theatre-style crawling lights.
//Timer callback

void TheaterChase() {
	 int i=0;
   if (ChaseCycle > 0)
   {
	 if  (TheaterChaseQ < 3 )
	 {
		 // erase previous

		 if (TheaterChaseQ!=0)
		 {
			 for (i=0; i < strip.numPixels(); i=i+3)
		                  strip.setPixelColor(i+TheaterChaseQ-1, 0);        //turn prev every third pixel off
		 }

         for (i=0; i < strip.numPixels(); i=i+3)
                        strip.setPixelColor(i+TheaterChaseQ, StripColor);    //turn every third pixel on
         strip.show();
	 }
	 else
	 {
	   ChaseCycle--;
	   TheaterChaseQ=0;
	   // erase last one
		 for (i=0; i < strip.numPixels(); i=i+3)
	                  strip.setPixelColor(i+2, 0);        //turn every third pixel off last time
	     strip.show();
	 }

   }
   else
   {
	   // finish this demo
		StripDemoType++;         // next demo type
		TheaterChaseTimer.stop();   // stop this demo dimer
		StripDemoTimer.initializeMs(2000, StartDemo).start(true);  // start another demo after 2 seconds
   }
}





//
// Demo timer callback
//

void StartDemo() {

	Serial.print("NeoPixel Demo type: ");
	Serial.println(StripDemoType);

	StripDemoTimer.stop();			 // next demo wait until this demo ends



   	StripNo = 0;   //start from led index 0
   	TheaterChaseQ =0; //another counter

	switch(StripDemoType){           // select demo type
    case 0:
    	    StripColor = strip.Color(0, 0, 0);	// black
    	   	ColorWipeTimer.initializeMs(50, ColorWipe).start(true);   // 50 ms step
    	   	break;
    case 1:
    		StripColor = strip.Color(255, 0, 0);	// Red
    		ColorWipeTimer.initializeMs(50, ColorWipe).start(true);   // 50 ms step
    		break;
    case 2:
		StripColor = strip.Color(0, 255, 0);	// Green
		ColorWipeTimer.initializeMs(100, ColorWipe).start(true);   // 100 ms step
            break;
    case 3:
		StripColor = strip.Color(0,0, 255);	// Blue
		ColorWipeTimer.initializeMs(150, ColorWipe).start(true);   // 150 ms step
            break;
    case 4:
    	    StripColor = strip.Color(0, 0, 0);	// black
    	   	ColorWipeTimer.initializeMs(50, ColorWipe).start(true);   // 50 ms step
    	   	break;

    case 5:
    	    ChaseCycle=10;	//do 10 cycles of chasing
		StripColor = strip.Color(127,127, 127);	// White
		TheaterChaseTimer.initializeMs(50, TheaterChase).start(true);   // 50 ms step
            break;
    case 6:
    	    ChaseCycle=15; //do 15 cycles of chasing
			StripColor = strip.Color(127,0,0);	// Red
			TheaterChaseTimer.initializeMs(60, TheaterChase).start(true);   // 60 ms step
            break;
    case 7:
   	        ChaseCycle=20; //do 20 cycles of chasing
			StripColor = strip.Color(0 , 0, 127);	// Blue
			TheaterChaseTimer.initializeMs(70, TheaterChase).start(true);   // 70 ms step
            break;

    default:
    	    StripDemoType=0;
    	    StripDemoTimer.initializeMs(1000, StartDemo).start(true);  //demo loop restart
    	    break;

  }

}



// Will be called when WiFi station was connected to AP
void connect_Ok()
{
	Serial.print("I'm CONNECTED - ");
	Serial.println(WifiStation.getIP().toString());

	//You can put here other job like web,tcp etc.
}

// Will be called when WiFi station timeout was reached
void connect_Fail()
{
	Serial.println("I'm NOT CONNECTED!");
	WifiStation.waitConnection(connect_Ok, 10, connect_Fail); // Repeat and check again
}

void init()
{

	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(false); // Disable debug output to serial

	Serial.print("NeoPixel demo .. start");


    // Wifi could be used eg. for switching Neopixel from internet
	// could be also dissabled if no needed

	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiStation.enable(true);
	WifiAccessPoint.enable(false);
	WifiStation.waitConnection(connect_Ok, 20, connect_Fail);



	StripDemoType =0;  //demo index to be displayed

	strip.begin();  //init port

	StripDemoTimer.initializeMs(1000, StartDemo).start();  //start demo


}



