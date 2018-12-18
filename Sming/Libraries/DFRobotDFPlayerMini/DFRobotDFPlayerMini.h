/*!
 * @file DFRobotDFPlayerMini.h
 * @brief DFPlayer - An Arduino Mini MP3 Player From DFRobot
 * @n Header file for DFRobot's DFPlayer
 *
 * @copyright	[DFRobot]( http://www.dfrobot.com ), 2016
 * @copyright	GNU Lesser General Public License
 *
 * @author [Angelo](Angelo.qiao@dfrobot.com)
 * @version  V1.0
 * @date  2016-12-07
 */

#include "Arduino.h"

#ifndef DFRobotDFPlayerMini_cpp
    #define DFRobotDFPlayerMini_cpp


#define DFPLAYER_EQ_NORMAL 0
#define DFPLAYER_EQ_POP 1
#define DFPLAYER_EQ_ROCK 2
#define DFPLAYER_EQ_JAZZ 3
#define DFPLAYER_EQ_CLASSIC 4
#define DFPLAYER_EQ_BASS 5

#define DFPLAYER_DEVICE_U_DISK 1
#define DFPLAYER_DEVICE_SD 2
#define DFPLAYER_DEVICE_AUX 3
#define DFPLAYER_DEVICE_SLEEP 4
#define DFPLAYER_DEVICE_FLASH 5

#define DFPLAYER_RECEIVED_LENGTH 10
#define DFPLAYER_SEND_LENGTH 10

//#define _DEBUG

#define TimeOut 0
#define WrongStack 1
#define DFPlayerCardInserted 2
#define DFPlayerCardRemoved 3
#define DFPlayerCardOnline 4
#define DFPlayerPlayFinished 5
#define DFPlayerError 6

#define Busy 1
#define Sleeping 2
#define SerialWrongStack 3
#define CheckSumNotMatch 4
#define FileIndexOut 5
#define FileMismatch 6
#define Advertise 7

#define Stack_Header 0
#define Stack_Version 1
#define Stack_Length 2
#define Stack_Command 3
#define Stack_ACK 4
#define Stack_Parameter 5
#define Stack_CheckSum 7
#define Stack_End 9

class DFRobotDFPlayerMini {
  Stream* _serial;
  
  unsigned long _timeOutTimer;
  unsigned long _timeOutDuration = 500;
  
  uint8_t _received[DFPLAYER_RECEIVED_LENGTH];
  uint8_t _sending[DFPLAYER_SEND_LENGTH] = {0x7E, 0xFF, 06, 00, 01, 00, 00, 00, 00, 0xEF};
  
  uint8_t _receivedIndex=0;

  void sendStack();
  void sendStack(uint8_t command);
  void sendStack(uint8_t command, uint16_t argument);
  void sendStack(uint8_t command, uint8_t argumentHigh, uint8_t argumentLow);

  void enableACK();
  void disableACK();
  
  void uint16ToArray(uint16_t value,uint8_t *array);
  
  uint16_t arrayToUint16(uint8_t *array);
  
  uint16_t calculateCheckSum(uint8_t *buffer);
  


  void parseStack();
  bool validateStack();
  
  uint8_t device = DFPLAYER_DEVICE_SD;
  
  public:
  
  uint8_t _handleType;
  uint8_t _handleCommand;
  uint16_t _handleParameter;
  bool _isAvailable = false;
  bool _isSending = false;
  
  bool handleMessage(uint8_t type, uint16_t parameter = 0);
  bool handleError(uint8_t type, uint16_t parameter = 0);

  uint8_t readCommand();
  
  bool begin(Stream& stream, bool isACK = true, bool doReset = true);
  
  bool waitAvailable();
  
  bool available();
  
  uint8_t readType();
  
  uint16_t read();
  
  void setTimeOut(unsigned long timeOutDuration);
  
  void next();
  
  void previous();
  
  void play(int fileNumber=1);
  
  void volumeUp();
  
  void volumeDown();
  
  void volume(uint8_t volume);
  
  void EQ(uint8_t eq);
  
  void loop(int fileNumber);
  
  void outputDevice(uint8_t device);
  
  void sleep();
  
  void reset();
  
  void start();
  
  void pause();
  
  void playFolder(uint8_t folderNumber, uint8_t fileNumber);
  
  void outputSetting(bool enable, uint8_t gain);
  
  void enableLoopAll();
  
  void disableLoopAll();
  
  void playMp3Folder(int fileNumber);
  
  void advertise(int fileNumber);
  
  void playLargeFolder(uint8_t folderNumber, uint16_t fileNumber);
  
  void stopAdvertise();
  
  void stop();
  
  void loopFolder(int folderNumber);
  
  void randomAll();
  
  void enableLoop();
  
  void disableLoop();
  
  void enableDAC();
  
  void disableDAC();
  
  int readState();
  
  int readVolume();
  
  uint8_t readEQ();
  
  int readFileCounts(uint8_t device);
  
  int readCurrentFileNumber(uint8_t device);
  
  int readFileCountsInFolder(int folderNumber);
  
  int readFileCounts();

  int readFolderCounts();
  
  int readCurrentFileNumber();
  
};

#endif
