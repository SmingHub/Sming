/*

This is an attiny84 example code for the nRF24L01 that can communicate with RF24 library

All the support files and libraries for the attiny for nRF24L01 is at repo listed  below

 *  repo   : https://github.com/stanleyseow/arduino-nrf24l01/
 *  Author : Stanley Seow
 *  e-mail : stanleyseow@gmail.com
 *  date   : 8 Aug 2013
 
 Some default values to take note when using this mirf/spi85 library
 Uses Mirf forked library from https://github.com/xdarklight/arduino-nrf24l01
 
 - node addressing is similar to RF24 libs but the bytes are flipped 
 
   byte TADDR[] = {0xe3, 0xf0, 0xf0, 0xf0, 0xf0}; will matches receiver node of RF24 below 
   const uint64_t pipes[2] = { 0x7365727631LL, 0xF0F0F0F0E3LL };
 
 The repo for the RF24 lib is at https://github.com/stanleyseow/RF24/

 Added TinyDebugSerial to the codes for TX only serial debugging
 https://code.google.com/p/arduino-tiny/

*/

#include <SPI85.h>
#include <Mirf.h>
#include <MirfHardwareSpi85Driver.h>
#include <TinyDebugSerial.h>
TinyDebugSerial mySerial = TinyDebugSerial(); // PB0 on attiny84

// This USI was defined in SPI85.cpp
// Not to be confused with SPI (MOSI/MISO) used by ICSP pins
// Refer to page 61 of attiny84 datahseet
//
//#define USI-DO  5
//#define USI-DI  4
//#define USCK   6

#define CE    7    
#define CSN   3 

// ATMEL ATTINY84 / ARDUINO
//
//                           +-\/-+
//                     VCC  1|    |14  GND
//  SerialTx   (D  0)  PB0  2|    |13  AREF (D 10)
//             (D  1)  PB1  3|    |12  PA1  (D  9) 
//  RESET              PB3  4|    |11  PA2  (D  8) 
//  PWM  INT0  (D  2)  PB2  5|    |10  PA3  (D  7)  CE
//  SS/CSN     (D  3)  PA7  6|    |9   PA4  (D  6)  USCK
//  USI-DI     (D  4)  PA6  7|    |8   PA5  (D  5)  USI-DO
//                           +----+

int bufferSize = 0;
char buffer[32] = "";
unsigned int counter = 0; 
uint8_t nodeID = 0;

void setup(){
  
  mySerial.begin( 9600 );    // for tiny_debug_serial 
  
  Mirf.cePin = CE;
  Mirf.csnPin = CSN;
  Mirf.spi = &MirfHardwareSpi85;
  Mirf.init();

  // This address is compatible with my example of rpi-hub or nRF24_Arduino_as_hub
  // at repo https://github.com/stanleyseow/RF24/examples/ 
  
  byte RADDR[] = {0xe7, 0xde, 0xde, 0xde, 0xde};
  byte TADDR[] = {0xe9, 0xde, 0xde, 0xde, 0xde};
 
  // Get nodeID from TXADDR 
  nodeID = *TADDR & 0xff; 
  
  // Compatible with RF24
  Mirf.baseConfig = _BV(EN_CRC) | _BV(CRCO);
  
  // RF_DR_LOW, RF_DR_HIGH
  // 00 - 1Mbps
  // 01 - 2Mbps
  // 10 - 250Kbps
  // 11 - Reserved
  // 1<<2 & 1<<1 is for Max RF Power
  Mirf.configRegister( RF_SETUP,( 1<<RF_DR_LOW | 0<<RF_DR_HIGH ) | 1<<2 | 1<<1 );
  
  Mirf.channel = 0x55; // Same as rpi-hub and sendto_hub ( channel 85 )
  
  Mirf.setRADDR(RADDR);  
  Mirf.setTADDR(TADDR);
  Mirf.config();
  
  // Enable dynamic payload on the other side
  Mirf.configRegister( FEATURE, 1<<EN_DPL ); 
  Mirf.configRegister( DYNPD, 1<<DPL_P0 | 1<<DPL_P1 | 1<<DPL_P2 | 1<<DPL_P3 | 1<<DPL_P4 | 1<<DPL_P5 ); 
  
  delay(100);
  
  // Print out register readinds for important settings
  uint8_t rf_ch, rf_setup = 0;
  byte tx_addr[5];
  byte rx_addr[5];
  
  Mirf.readRegister(RF_CH, &rf_ch,sizeof(rf_ch));
  Mirf.readRegister(RF_SETUP, &rf_setup, sizeof(rf_setup));
  Mirf.readRegister(TX_ADDR, tx_addr, sizeof(tx_addr));
  Mirf.readRegister(RX_ADDR_P1, rx_addr, sizeof(rx_addr));
    
  mySerial.println();
  
  mySerial.print("RF_CH :0x");
  mySerial.println(rf_ch,HEX);  
  
  mySerial.print("RF_SETUP :0x");
  mySerial.println(rf_setup,HEX);  
  
  mySerial.print("TX_ADDR :");
  for ( int i=0;i<5;i++ ) {  // Loop 5 times, print in HEX
  mySerial.print( tx_addr[i], HEX);
  }
  mySerial.println();
  
  mySerial.print("RX_ADDR :");
  for ( int i=0;i<5;i++ ) {  // Loop 5 times, print in HEX
  mySerial.print( rx_addr[i], HEX);
  }
  mySerial.println();
  
  delay(1000);      // For serial debug to read the init config output
}

void loop(){
  
  uint8_t sent = false;
  unsigned long timer1;

  timer1 = millis();
  counter++;

  sprintf(buffer,"%2X,%u,%08lu",nodeID,counter,timer1);
  Mirf.payload = strlen(buffer);

  mySerial.print("Len :");
  mySerial.print(strlen(buffer));
  mySerial.print(" Buffer :");  
  mySerial.println(buffer);
 
  Mirf.send((byte *) buffer);
  
  while( Mirf.isSending() )
  {
    delay(1);
    sent = true; // Sent success
  }
  
  if (sent) {   
        mySerial.print("Sent :");
        mySerial.println(buffer);
    } else {
    }
    
    delay(1000);
} // End loop()


