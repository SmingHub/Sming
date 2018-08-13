/*
 * HexDump.h
 *
 *  Created on: Feb 14, 2016
 *      Author: harry
 */

#ifndef SAMPLES_ARDUCAM_APP_HEXDUMP_H_
#define SAMPLES_ARDUCAM_APP_HEXDUMP_H_


class HexDump {
public:
	HexDump();
	virtual ~HexDump();

	void print(unsigned char *data, int len);
	void resetAddr();
private:
	void pritln(char * buf, int len);
//	int width = 16;
	int addr = 0;
//	char *buf = new char(16);

};

#endif /* SAMPLES_ARDUCAM_APP_HEXDUMP_H_ */
