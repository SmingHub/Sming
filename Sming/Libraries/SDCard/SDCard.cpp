/*
Modified by: (github.com/)ADiea
Project: Sming for ESP8266 - https://github.com/anakod/Sming
License: MIT
Date: 15.07.2015
Descr: Low-level SDCard functions
*/
/*------------------------------------------------------------------------/
/  Foolproof MMCv3/SDv1/SDv2 (in SPI mode) control module
/-------------------------------------------------------------------------/
/
/  Copyright (C) 2013, ChaN, all right reserved.
/
/ * This software is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/-------------------------------------------------------------------------/
  Features and Limitations:

  * Easy to Port Bit-banging SPI
    It uses only four GPIO pins. No complex peripheral needs to be used.

  * Platform Independent
    You need to modify only a few macros to control the GPIO port.

  * Low Speed
    The data transfer rate will be several times slower than hardware SPI.

  * No Media Change Detection
    Application program needs to perform a f_mount() after media change.

/-------------------------------------------------------------------------*/
#include "SDCard.h"
#include "fatfs/ff.h"
#include "fatfs/diskio.h" /* Declarations of disk I/O functions */
#include <Clock.h>
#include <debug_progmem.h>

SPIBase* SDCardSPI;

namespace
{
FATFS* pFatFs;		  /* FatFs work area needed for each volume */
uint8_t chipSelect;   /* SPI client selector */
uint32_t spiInitFreq; /* SPI frequency used for initialisation */

}; // namespace

bool SDCard_begin(uint8_t slaveSelect, uint32_t freqLimit)
{
	chipSelect = slaveSelect;
	digitalWrite(chipSelect, HIGH);
	pinMode(chipSelect, OUTPUT);
	digitalWrite(chipSelect, HIGH);

	if(SDCardSPI == nullptr) {
		debug_e("SDCard SPI object not created.");
		return false;
	}

	auto& settings = SDCardSPI->SPIDefaultSettings;

	settings.byteOrder = LSBFIRST;
	settings.dataMode = SPI_MODE0;

	spiInitFreq = 1000000U;
	if(freqLimit > 0 && freqLimit < spiInitFreq) {
		spiInitFreq = freqLimit;
	}

	// Note: Some SD cards struggle at 40MHz
	settings.speed = std::min(freqLimit, 40000000U);

	if(!SDCardSPI->begin()) {
		debug_e("SDCard SPI init failed");
		return false;
	}

	/* This must be allocated for the whole program life ~512Bytes*/
	pFatFs = new FATFS;
	if(pFatFs == nullptr) {
		debug_e("No heap for pFatFs");
		return false;
	}

	/* Give a work area to the default drive */
	FRESULT mountRes = f_mount(pFatFs, "", 0);
	if(FR_OK != mountRes) {
		debug_e("f_mount: FAIL %d", mountRes);
		return false;
	}

	/* open dummy file to force card init */
	FIL file;
	if(FR_OK == f_open(&file, "dummy", FA_READ)) {
		f_close(&file);
	}

	return true;
}

namespace
{
/*-------------------------------------------------------------------------*/
/* Platform dependent macros and functions needed to be modified           */
/*-------------------------------------------------------------------------*/
void dly_us(UINT n) /* Delay n microseconds (avr-gcc -Os) */
{
	delayMicroseconds(n);
}

/*--------------------------------------------------------------------------

   Module Private Functions

---------------------------------------------------------------------------*/

/* MMC/SD command (SPI mode) */
#define CMD0 (0)		   /* GO_IDLE_STATE */
#define CMD1 (1)		   /* SEND_OP_COND */
#define ACMD41 (0x80 + 41) /* SEND_OP_COND (SDC) */
#define CMD8 (8)		   /* SEND_IF_COND */
#define CMD9 (9)		   /* SEND_CSD */
#define CMD10 (10)		   /* SEND_CID */
#define CMD12 (12)		   /* STOP_TRANSMISSION */
#define CMD13 (13)		   /* SEND_STATUS */
#define ACMD13 (0x80 + 13) /* SD_STATUS (SDC) */
#define CMD16 (16)		   /* SET_BLOCKLEN */
#define CMD17 (17)		   /* READ_SINGLE_BLOCK */
#define CMD18 (18)		   /* READ_MULTIPLE_BLOCK */
#define CMD23 (23)		   /* SET_BLOCK_COUNT */
#define ACMD23 (0x80 + 23) /* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24 (24)		   /* WRITE_BLOCK */
#define CMD25 (25)		   /* WRITE_MULTIPLE_BLOCK */
#define CMD32 (32)		   /* ERASE_ER_BLK_START */
#define CMD33 (33)		   /* ERASE_ER_BLK_END */
#define CMD38 (38)		   /* ERASE */
#define CMD55 (55)		   /* APP_CMD */
#define CMD58 (58)		   /* READ_OCR */

DSTATUS diskStatus{STA_NOINIT};

BYTE CardType; /* b0:MMC, b1:SDv1, b2:SDv2, b3:Block addressing */

/*-----------------------------------------------------------------------*/
/* Wait for card ready                                                   */
/*-----------------------------------------------------------------------*/

bool wait_ready() /* 1:OK, 0:Timeout */
{
	for(unsigned tmr = 5000; tmr; tmr--) { /* Wait for ready in timeout of 500ms */
		BYTE d = SDCardSPI->transfer(0xff);
		if(d == 0xFF) {
			return true;
		}
		dly_us(100);
	}

	return false;
}

/*-----------------------------------------------------------------------*/
/* Deselect the card and release SPI bus                                 */
/*-----------------------------------------------------------------------*/

void deselect()
{
	digitalWrite(chipSelect, HIGH);
	SDCardSPI->transfer(0xff); /* Send 0xFF Dummy clock (force DO hi-z for multiple slave SPI) */
}

/*-----------------------------------------------------------------------*/
/* Select the card and wait for ready                                    */
/*-----------------------------------------------------------------------*/

bool select() /* 1:OK, 0:Timeout */
{
	digitalWrite(chipSelect, LOW);
	SDCardSPI->transfer(0xff); /* Dummy clock (force DO enabled) */
	if(wait_ready()) {
		return true;
	}

	debug_e("SDCard select() failed");
	deselect();
	return false;
}

/*-----------------------------------------------------------------------*/
/* Receive a data packet from the card                                   */
/*-----------------------------------------------------------------------*/

bool rcvr_datablock(			/* 1:OK, 0:Failed */
					BYTE* buff, /* Data buffer to store received data */
					UINT btr	/* Byte count */
)
{
	/* Wait for data packet in timeout of 100ms */
	BYTE d{0xFF};
	for(unsigned tmr = 1000; tmr; tmr--) {
		d = SDCardSPI->transfer(0xff);
		if(d != 0xFF) {
			break;
		}
		dly_us(100);
	}
	if(d != 0xFE) {
		return false; /* If not valid data token, return with error */
	}

	memset(buff, 0xFF, btr);		/* Send 0xFF */
	SDCardSPI->transfer(buff, btr); /* Receive the data block into buffer */
	SDCardSPI->transfer16(0xffff);  /* keep MOSI HIGH, discard CRC */

	// success
	return true;
}

/*-----------------------------------------------------------------------*/
/* Send a data packet to the card                                        */
/*-----------------------------------------------------------------------*/

bool xmit_datablock(				  /* 1:OK, 0:Failed */
					const BYTE* buff, /* 512 byte data block to be transmitted */
					BYTE token		  /* Data/Stop token */
)
{
	if(!wait_ready()) {
		debug_e("[SDCard] wait_ready failed");
		return false;
	}

	SDCardSPI->transfer(token); /* Xmit a token */
	if(token != 0xFD) {			/* Is it data token? */
		// Data gets modified so take a copy
		uint8_t buffer[512];
		memcpy(buffer, buff, sizeof(buffer));
		SDCardSPI->transfer(buffer, sizeof(buffer)); /* Xmit the 512 byte data block to MMC */

		//		SDCardSPI->setMOSI(HIGH); /* Send 0xFF */
		SDCardSPI->transfer16(0xffff);		/* Xmit dummy CRC */
		BYTE d = SDCardSPI->transfer(0xff); /* keep MOSI HIGH and receive data response */

		if((d & 0x1F) != 0x05) { /* If not accepted, return with error */
			debug_e("[SDCard] data not accepted, d = 0x%02x", d);
			return false;
		}
	}

	return true;
}

/*-----------------------------------------------------------------------*/
/* Send a command packet to the card                                     */
/*-----------------------------------------------------------------------*/

BYTE send_cmd(			/* Returns command response (bit7==1:Send failed)*/
			  BYTE cmd, /* Command byte */
			  DWORD arg /* Argument */
)
{
	if(cmd & 0x80) { /* ACMD<n> is the command sequence of CMD55-CMD<n> */
		cmd &= 0x7F;
		BYTE n = send_cmd(CMD55, 0);
		if(n > 1) {
			debug_e("[SDCard] CMD55 error, n = 0x%02x", n);
			return n;
		}
	}

	/* Select the card and wait for ready except to stop multiple block read */
	if(cmd != CMD12) {
		deselect();
		if(!select()) {
			debug_e("[SDCard] Select failed");
			return 0xFF;
		}
	}

	/* Send a command packet */
	BYTE crc;
	if(cmd == CMD0) {
		crc = 0x95; /* (valid CRC for CMD0(0)) */
	} else if(cmd == CMD8) {
		crc = 0x87; /* (valid CRC for CMD8(0x1AA)) */
	} else {
		crc = 0x01; /* Dummy CRC + Stop */
	}
	BYTE buf[]{
		BYTE(0x40 | cmd), /* Start + Command index */
		BYTE(arg >> 24),  /* Argument[31..24] */
		BYTE(arg >> 16),  /* Argument[23..16] */
		BYTE(arg >> 8),   /* Argument[15..8] */
		BYTE(arg),		  /* Argument[7..0] */
		crc,
		0xff, /* Dummy clock (force DO enabled) */
	};
	SDCardSPI->transfer(buf, sizeof(buf));

	/* Receive command response */
	if(cmd == CMD12) {
		SDCardSPI->transfer(0xff); /* Skip a stuff byte when stop reading */
	}

	/* Wait for a valid response */
	BYTE d;
	unsigned n = 10;
	do {
		d = SDCardSPI->transfer(0xff);
	} while((d & 0x80) && --n);

	//	debug_i("SDcard send_cmd %u -> %u (%u try)", cmd, d, n);
	return d; /* Return with the response value */
}

} // namespace

/*--------------------------------------------------------------------------

   Public Functions

---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status(BYTE drv /* Drive number (always 0) */
)
{
	if(drv != 0) {
		return STA_NOINIT;
	}

	return diskStatus;
}

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(BYTE drv /* Physical drive nmuber (0) */
)
{
	if(drv != 0) {
		return RES_NOTRDY;
	}

	SPISettings initSettings(spiInitFreq, LSBFIRST, SPI_MODE0);
	SDCardSPI->beginTransaction(initSettings);

	dly_us(10000);

	debug_i("disk_initialize: send 80 0xFF cycles");
	BYTE tmp[80 / 8];
	memset(tmp, 0xff, sizeof(tmp));
	SDCardSPI->transfer(tmp, sizeof(tmp));

	//	debug_i("disk_initialize: send n send_cmd(CMD0, 0)");
	BYTE retCmd;
	BYTE n = 5;
	do {
		retCmd = send_cmd(CMD0, 0);
		n--;
	} while(n && retCmd != 1);
	debug_i("disk_initialize: until n = 5 && ret != 1");

	BYTE ty = 0;
	if(retCmd == 1) {
		debug_i("disk_initialize: Enter Idle state, send_cmd(CMD8, 0x1AA) == 1");
		/* Enter Idle state */
		if(send_cmd(CMD8, 0x1AA) == 1) { /* SDv2? */
			debug_i("[SDCard] Sdv2 ?");
			BYTE buf[4]{0xff, 0xff, 0xff, 0xff};
			SDCardSPI->transfer(buf, sizeof(buf));
			debug_hex(INFO, "[SDCard]", buf, sizeof(buf));
			if(buf[2] == 0x01 && buf[3] == 0xAA) { /* The card can work at vdd range of 2.7-3.6V */
				UINT tmr;
				for(tmr = 1000; tmr; tmr--) { /* Wait for leaving idle state (ACMD41 with HCS bit) */
					if(send_cmd(ACMD41, 1UL << 30) == 0) {
						debug_i("[SDCard] ACMD41 OK");
						break;
					}
					dly_us(1000);
				}
				if(tmr == 0) {
					debug_i("[SDCard] ACMD41 FAIL");
				}
				if(tmr != 0 && send_cmd(CMD58, 0) == 0) { /* Check CCS bit in the OCR */
														  //					SDCardSPI->setMOSI(HIGH); /* Send 0xFF */
														  //					SDCardSPI->recv(buf, 4);
					//					ty = (buf[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;	/* SDv2 */
					memset(buf, 0xFF, sizeof(buf));
					SDCardSPI->transfer(buf, sizeof(buf));
					ty = (buf[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2; /* SDv2 */
					debug_hex(INFO, "[SDCard]", buf, sizeof(buf));
				}
			}
		} else { /* SDv1 or MMCv3 */
			debug_i("[SDCard] Sdv1 / MMCv3 ?");
			BYTE cmd;
			if(send_cmd(ACMD41, 0) <= 1) {
				ty = CT_SD1;
				cmd = ACMD41; /* SDv1 */
			} else {
				ty = CT_MMC;
				cmd = CMD1; /* MMCv3 */
			}
			UINT tmr;
			for(tmr = 1000; tmr; tmr--) { /* Wait for leaving idle state */
				if(send_cmd(cmd, 0) == 0) {
					break;
				}
				dly_us(1000);
			}
			if(tmr == 0 || send_cmd(CMD16, 512) != 0) /* Set R/W block length to 512 */
			{
				debug_i("[SDCard] tmr = 0 || CMD16 != 0");
				ty = 0;
			}
		}
	} else {
		debug_e("SDCard ERROR: %x", retCmd);
	}
	CardType = ty;

	if(ty == 0) {
		diskStatus = STA_NOINIT;
		debug_e("SDCard init FAIL");
	} else {
		diskStatus = 0;
		debug_i("SDCard OK: TYPE %u", ty);
	}

	deselect();

	SDCardSPI->beginTransaction(SDCardSPI->SPIDefaultSettings);

	return diskStatus;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read(BYTE drv,		/* Physical drive nmuber (0) */
				  BYTE* buff,   /* Pointer to the data buffer to store read data */
				  DWORD sector, /* Start sector number (LBA) */
				  UINT count	/* Sector count (1..128) */
)
{
	if(disk_status(drv) & STA_NOINIT) {
		return RES_NOTRDY;
	}
	if(!(CardType & CT_BLOCK)) {
		sector *= 512; /* Convert LBA to byte address if needed */
	}

	BYTE cmd = count > 1 ? CMD18 : CMD17; /*  READ_MULTIPLE_BLOCK : READ_SINGLE_BLOCK */
	if(send_cmd(cmd, sector) == 0) {
		do {
			if(!rcvr_datablock(buff, 512)) {
				debug_e("[SDCard] rcvr error");
				break;
			}
			buff += 512;
		} while(--count);
		if(cmd == CMD18) {
			send_cmd(CMD12, 0); /* STOP_TRANSMISSION */
		}
	}
	deselect();

	return count ? RES_ERROR : RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write(BYTE drv,		 /* Physical drive nmuber (0) */
				   const BYTE* buff, /* Pointer to the data to be written */
				   DWORD sector,	 /* Start sector number (LBA) */
				   UINT count		 /* Sector count (1..128) */
)
{
	if(disk_status(drv) & STA_NOINIT) {
		return RES_NOTRDY;
	}
	if(!(CardType & CT_BLOCK)) {
		sector *= 512; /* Convert LBA to byte address if needed */
	}

	if(count == 1) {					  /* Single block write */
		if((send_cmd(CMD24, sector) == 0) /* WRITE_BLOCK */
		   && xmit_datablock(buff, 0xFE)) {
			count = 0;
		} else {
			debug_e("[SDCard] CMD24 error");
		}
	} else { /* Multiple block write */
		if(CardType & CT_SDC) {
			send_cmd(ACMD23, count);
		}
		if(send_cmd(CMD25, sector) == 0) { /* WRITE_MULTIPLE_BLOCK */
			do {
				if(!xmit_datablock(buff, 0xFC)) {
					debug_e("[SDCard] xmit error");
					break;
				}
				buff += 512;
			} while(--count);
			if(!xmit_datablock(0, 0xFD)) { /* STOP_TRAN token */
				debug_e("[SDCard] STOP_TRAN error");
				count = 1;
			}
		}
	}
	deselect();

	return count ? RES_ERROR : RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl(BYTE drv,  /* Physical drive nmuber (0) */
				   BYTE ctrl, /* Control code */
				   void* buff /* Buffer to send/receive control data */
)
{
	if(disk_status(drv) & STA_NOINIT) {
		return RES_NOTRDY; /* Check if card is in the socket */
	}

	DRESULT res = RES_ERROR;
	switch(ctrl) {
	case CTRL_SYNC: /* Make sure that no pending write process */
		if(select()) {
			res = RES_OK;
		}
		break;

	case GET_SECTOR_COUNT: { /* Get number of sectors on the disk (DWORD) */
		BYTE csd[16];
		if((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16)) {
			DWORD dw;
			if((csd[0] >> 6) == 1) { /* SDC ver 2.00 */
				DWORD cs = csd[9] + ((WORD)csd[8] << 8) + ((DWORD)(csd[7] & 63) << 16) + 1;
				dw = cs << 10;
			} else { /* SDC ver 1.XX or MMC */
				BYTE n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
				DWORD cs = (csd[8] >> 6) + ((WORD)csd[7] << 2) + ((WORD)(csd[6] & 3) << 10) + 1;
				dw = cs << (n - 9);
			}
			memcpy(buff, &dw, sizeof(dw));
			res = RES_OK;
		}
		break;
	}

	case GET_BLOCK_SIZE: { /* Get erase block size in unit of sector (DWORD) */
		DWORD dw{128};
		memcpy(buff, &dw, sizeof(dw));
		res = RES_OK;
		break;
	}

	default:
		res = RES_PARERR;
	}

	deselect();

	return res;
}
