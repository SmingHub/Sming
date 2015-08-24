/* 
 * File:   ESP8266_SPI.h
 * Author: chain-admin
 *
 * Created on July 25, 2015, 7:06 PM
 */

#ifndef ESP8266_SPI_H
#define	ESP8266_SPI_H
#include "../include/user_config.h"
#include <../SmingCore/SmingCore.h>

//Define SPI hardware modules
//#define SPI 0
#define HSPI 1

#define SPI_CLK_USE_DIV 0
#define SPI_CLK_80MHZ_NODIV 1

#define SPI_BYTE_ORDER_HIGH_TO_LOW 1
#define SPI_BYTE_ORDER_LOW_TO_HIGH 0

#ifndef CPU_CLK_FREQ //Should already be defined in eagle_soc.h
#define CPU_CLK_FREQ 80*1000000
#endif

//Define some default SPI clock settings
#define SPI_CLK_PREDIV 10
#define SPI_CLK_CNTDIV 2
#define SPI_CLK_FREQ CPU_CLK_FREQ/(SPI_CLK_PREDIV*SPI_CLK_CNTDIV) // 80 / 20 = 4 MHz



class HwSPIClass {
public:
	HwSPIClass();

	void begin(uint8_t spi_no); // Default
	void end(uint8_t spi_no);

	void transfer(uint8_t * data, uint8_t count);
	byte transfer(uint8_t data);
	
        void spi_init_gpio(uint8_t spi_no, uint8_t sysclk_as_spiclk);
        void spi_clock(uint8_t spi_no, uint16 prediv, uint8_t cntdiv);
        void spi_tx_byte_order(uint8_t spi_no, uint8_t byte_order);
        void spi_rx_byte_order(uint8_t spi_no, uint8_t byte_order);
        void spi_init(uint8_t spi_no);
        uint32 spi_transaction(uint8_t spi_no, uint8_t cmd_bits, uint16 cmd_data, uint32 addr_bits, uint32 addr_data, uint32 dout_bits, uint32 dout_data, uint32 din_bits, uint32 dummy_bits);

        void spi_txd(uint8_t spi_no, uint8_t bits, uint32 data); 
        void spi_tx8(uint8_t spi_no, uint8_t data);    
        void spi_tx16(uint8_t spi_no, uint16 data) ;     
        void spi_tx32(uint8_t spi_no, uint32 data) ;     
        uint32 spi_rxd(uint8_t spi_no, uint8_t bits); 
        uint8_t spi_rx8(uint8_t spi_no);       
        uint16 spi_rx16(uint8_t spi_no);     
        uint32 spi_rx32(uint8_t spi_no);     

protected:
	int spi_busy(uint8_t spi_no) ;
private:
	uint8_t id;
	//uint8_t order; // = MSBFIRST
};

extern HwSPIClass HwSPI;

#endif	/* ESP8266_SPI_H */

