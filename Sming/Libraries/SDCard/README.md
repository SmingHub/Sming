# SD Card

Low-level support code for accessing SD Cards using FATFS.

I couldn't get SDCard to work on a Wemos D1 mini with an SD Card adaptor. Always gave the same error. SD Card Error: FF.
I hooked up the scope and discovered that the SD Card was receiving SPI messages but never responsed.
I hooked up the logic analyser to analyse the problem. Had to slow down the SPI to allow my analyser to work.
Image SPI_MSBFIRST.jpg is the result.
You can see first 80 dummy clock cycles; then 2 dummy reads, then the 6 byte CMD0 which is required to initialise the SDcard. 
Then 11 dummy reads while the host waits for a response from the SD card. No response so the cycle repeats (not shown)
The problem is the order of the bytes. The correct command is 40 00 00 00 00 95 (command ID, 32 attribute bits, checksum)
What is actually shown is 00 00 00 40 95 00. The byte order is wrong on a 32 bit boundary.

The fix is to initialise SPI with byte order set to LSBFISRT.

Image SPI_LSBFIRST.jpg shows the 80 dummy clock cycles, followed by 2 dummy reads then the correct CMD0. Now the SD responds with 01. Which prompts the host to 
send CMD8 requesting the card type. Everything works fine now.

I changed sdcard.cpp SDCard_begin function to take byte order and frequency limit as variables [SDCard_begin(uint8 PIN_CARD_SS, uint8 byte_order, uint32 freq_limit)]
This allows application.cpp to choose the byte order as well as set frequency limit.
Frequency limit is useful because from various related posts, it appears some configurations become unreliable at 40MHz.
It also allows me an easy way to slow things down so my logic analyser will work.

The other change was to change the test file that is written and read from 'test' to 'test.txt'. The former could be written but not read.
No idea why but test.txt works for both read and write

Ray
