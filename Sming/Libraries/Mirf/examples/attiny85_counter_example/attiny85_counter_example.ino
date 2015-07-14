/**
 * A Mirf example test sending and receiving data between
 * Ardunio (running as server) and ATTiny85 (running as client).
 *
 * This uses the SPI85 class from: https://github.com/stanleyseow/attiny-nRF24L01/tree/master/libraries/SPI85
 *
 * Pins:
 * Hardware SPI:
 * Attiny x5 Datasheet page 62
 * USI-DI -> PB0 ( MISO )
 * USI-DO -> PB1 ( MOSI )
 * USI-SCL -> PB2 ( SCK )
 *
 * (Configurable):
 * CE -> PB4
 * CSN -> PB3
 */

// ATtiny25/45/85 Pin map
//                                 +-\/-+
//                Reset/Ain0 (D 5) PB5  1|o   |8  Vcc
//  nRF24L01 CE, Pin3 - Ain3 (D 3) PB3  2|    |7  PB2 (D 2) Ain1 - nRF24L01 SCK, pin5
// nRF24L01 CSN, Pin4 - Ain2 (D 4) PB4  3|    |6  PB1 (D 1) pwm1 - nRF24L01 MOSI, pin7
//                                 GND  4|    |5  PB0 (D 0) pwm0 - nRF24L01 MISO, pin6

#include <SPI85.h>
#include <Mirf.h>
#include <nRF24L01.h>
#include <MirfHardwareSpi85Driver.h>

void setup() {
  /*
   * Setup pins / SPI.
   */

  Mirf.cePin = PB4;
  Mirf.csnPin = PB3;

  Mirf.spi = &MirfHardwareSpi85;
  Mirf.init();
  
  /*
   * Configure reciving address.
   */
   
  Mirf.setRADDR((byte *)"clie1");
  
  /*
   * Set the payload length to sizeof(unsigned long) the
   * return type of millis().
   *
   * NB: payload on client and server must be the same.
   */
   
  Mirf.payload = sizeof(unsigned long);
  
  /*
   * Write channel and payload config then power up reciver.
   */
   
  /*
   * To change channel:
   * 
   * Mirf.channel = 10;
   *
   * NB: Make sure channel is legal in your area.
   */
   
  Mirf.config();
}

void loop() {
  static unsigned long counter = 0;
  unsigned long time = millis();
  
  Mirf.setTADDR((byte *)"serv1");
  
  Mirf.send((byte *)&counter);

  while (Mirf.isSending()) {
    if ((millis() - time) > 1000) {
      delay(1000);
      return;
    }
  }

  delay(10);

  while (!Mirf.dataReady()){
    if ((millis() - time) > 1000) {
      delay(1000);
      return;
    }
  }

  unsigned long recv;
  Mirf.getData((byte *) &recv);

  counter = recv + 1;

  delay(500);
} 
  
  
  
