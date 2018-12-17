/*!
 * @file DFRobotDFPlayerMini.cpp
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

#include "DFRobotDFPlayerMini.h"

void DFRobotDFPlayerMini::setTimeOut(unsigned long timeOutDuration){
  _timeOutDuration = timeOutDuration;
}

void DFRobotDFPlayerMini::uint16ToArray(uint16_t value, uint8_t *array){
  *array = (uint8_t)(value>>8);
  *(array+1) = (uint8_t)(value);
}

uint16_t DFRobotDFPlayerMini::calculateCheckSum(uint8_t *buffer){
  uint16_t sum = 0;
  for (int i=Stack_Version; i<Stack_CheckSum; i++) {
    sum += buffer[i];
  }
  return -sum;
}

void DFRobotDFPlayerMini::sendStack(){
  if (_sending[Stack_ACK]) {
    while (_isSending) {
      delay(0);
      available();
    }
  }
  else{
    delay(10);
  }

#ifdef _DEBUG
  Serial.println();
  Serial.print(F("sending:"));
  for (int i=0; i<DFPLAYER_SEND_LENGTH; i++) {
    Serial.print(_sending[i],HEX);
    Serial.print(F(" "));
  }
  Serial.println();
#endif
  _serial->write(_sending, DFPLAYER_SEND_LENGTH);
  _timeOutTimer = millis();
  _isSending = _sending[Stack_ACK];
}

void DFRobotDFPlayerMini::sendStack(uint8_t command){
  sendStack(command, 0);
}

void DFRobotDFPlayerMini::sendStack(uint8_t command, uint16_t argument){
  _sending[Stack_Command] = command;
  uint16ToArray(argument, _sending+Stack_Parameter);
  uint16ToArray(calculateCheckSum(_sending), _sending+Stack_CheckSum);
  sendStack();
}

void DFRobotDFPlayerMini::sendStack(uint8_t command, uint8_t argumentHigh, uint8_t argumentLow){
  uint16_t buffer = argumentHigh;
  buffer <<= 8;
  sendStack(command, buffer | argumentLow);
}

void DFRobotDFPlayerMini::enableACK(){
  _sending[Stack_ACK] = 0x01;
}

void DFRobotDFPlayerMini::disableACK(){
  _sending[Stack_ACK] = 0x00;
}

bool DFRobotDFPlayerMini::waitAvailable(){
  _isSending = true;
  while (!available()){
    delay(0);
  }
  return _handleType != TimeOut;
}

bool DFRobotDFPlayerMini::begin(Stream &stream, bool isACK, bool doReset){
  if (isACK) {
    enableACK();
  }
  else{
    disableACK();
  }
  
  _serial = &stream;

  if (doReset) {
    _timeOutDuration += 3000;
    reset();
    waitAvailable();
    _timeOutDuration -= 3000;
    delay(200);
  } else {
    // assume same state as with reset(): online
    _handleType = DFPlayerCardOnline;
  }

  return (readType() == DFPlayerCardOnline) || !isACK;
}

uint8_t DFRobotDFPlayerMini::readType(){
  _isAvailable = false;
  return _handleType;
}

uint16_t DFRobotDFPlayerMini::read(){
  _isAvailable = false;
  return _handleParameter;
}

bool DFRobotDFPlayerMini::handleMessage(uint8_t type, uint16_t parameter){
  _receivedIndex = 0;
  _handleType = type;
  _handleParameter = parameter;
  _isAvailable = true;
  return _isAvailable;
}

bool DFRobotDFPlayerMini::handleError(uint8_t type, uint16_t parameter){
  handleMessage(type, parameter);
  _isSending = false;
  return false;
}

uint8_t DFRobotDFPlayerMini::readCommand(){
  _isAvailable = false;
  return _handleCommand;
}

void DFRobotDFPlayerMini::parseStack(){
  _handleCommand = *(_received + Stack_Command);
  _handleParameter =  arrayToUint16(_received + Stack_Parameter);

  switch (_handleCommand) {
    case 0x3D:
      handleMessage(DFPlayerPlayFinished, _handleParameter);
      break;
    case 0x3F:
      if (_handleParameter & 0x02) {
        handleMessage(DFPlayerCardOnline, _handleParameter);
      }
      break;
    case 0x3A:
      if (_handleParameter & 0x02) {
        handleMessage(DFPlayerCardInserted, _handleParameter);
      }
      break;
    case 0x3B:
      if (_handleParameter & 0x02) {
        handleMessage(DFPlayerCardRemoved, _handleParameter);
      }
      break;
    case 0x40:
      handleMessage(DFPlayerError, _handleParameter);
      break;
    case 0x41:
      _isSending = false;
      break;
    case 0x3C:
    case 0x3E:
    case 0x42:
    case 0x43:
    case 0x44:
    case 0x45:
    case 0x46:
    case 0x47:
    case 0x48:
    case 0x49:
    case 0x4B:
    case 0x4C:
    case 0x4D:
    case 0x4E:
    case 0x4F:
      _isAvailable = true;
      break;
    default:
      handleError(WrongStack);
      break;
  }
}

uint16_t DFRobotDFPlayerMini::arrayToUint16(uint8_t *array){
  uint16_t value = *array;
  value <<=8;
  value += *(array+1);
  return value;
}

bool DFRobotDFPlayerMini::validateStack(){
  return calculateCheckSum(_received) == arrayToUint16(_received+Stack_CheckSum);
}

bool DFRobotDFPlayerMini::available(){
  while (_serial->available()) {
    delay(0);
    if (_receivedIndex == 0) {
      _received[Stack_Header] = _serial->read();
#ifdef _DEBUG
      Serial.print(F("received:"));
      Serial.print(_received[_receivedIndex],HEX);
      Serial.print(F(" "));
#endif
      if (_received[Stack_Header] == 0x7E) {
        _isAvailable = false;
        _receivedIndex ++;
      }
    }
    else{
      _received[_receivedIndex] = _serial->read();
#ifdef _DEBUG
      Serial.print(_received[_receivedIndex],HEX);
      Serial.print(F(" "));
#endif
      switch (_receivedIndex) {
        case Stack_Version:
          if (_received[_receivedIndex] != 0xFF) {
            return handleError(WrongStack);
          }
          break;
        case Stack_Length:
          if (_received[_receivedIndex] != 0x06) {
            return handleError(WrongStack);
          }
          break;
        case Stack_End:
#ifdef _DEBUG
          Serial.println();
#endif
          if (_received[_receivedIndex] != 0xEF) {
            return handleError(WrongStack);
          }
          else{
            if (validateStack()) {
              _receivedIndex = 0;
              parseStack();
              if (_isAvailable && !_sending[Stack_ACK]) {
                _isSending = false;
              }
              return _isAvailable;
            }
            else{
              return handleError(WrongStack);
            }
          }
          break;
        default:
          break;
      }
      _receivedIndex++;
    }
  }
  
  if (_isSending && (millis()-_timeOutTimer>=_timeOutDuration)) {
    return handleError(TimeOut);
  }
  
  return _isAvailable;
}

void DFRobotDFPlayerMini::next(){
  sendStack(0x01);
}

void DFRobotDFPlayerMini::previous(){
  sendStack(0x02);
}

void DFRobotDFPlayerMini::play(int fileNumber){
  sendStack(0x03, fileNumber);
}

void DFRobotDFPlayerMini::volumeUp(){
  sendStack(0x04);
}

void DFRobotDFPlayerMini::volumeDown(){
  sendStack(0x05);
}

void DFRobotDFPlayerMini::volume(uint8_t volume){
  sendStack(0x06, volume);
}

void DFRobotDFPlayerMini::EQ(uint8_t eq) {
  sendStack(0x07, eq);
}

void DFRobotDFPlayerMini::loop(int fileNumber) {
  sendStack(0x08, fileNumber);
}

void DFRobotDFPlayerMini::outputDevice(uint8_t device) {
  sendStack(0x09, device);
  delay(200);
}

void DFRobotDFPlayerMini::sleep(){
  sendStack(0x0A);
}

void DFRobotDFPlayerMini::reset(){
  sendStack(0x0C);
}

void DFRobotDFPlayerMini::start(){
  sendStack(0x0D);
}

void DFRobotDFPlayerMini::pause(){
  sendStack(0x0E);
}

void DFRobotDFPlayerMini::playFolder(uint8_t folderNumber, uint8_t fileNumber){
  sendStack(0x0F, folderNumber, fileNumber);
}

void DFRobotDFPlayerMini::outputSetting(bool enable, uint8_t gain){
  sendStack(0x10, enable, gain);
}

void DFRobotDFPlayerMini::enableLoopAll(){
  sendStack(0x11, 0x01);
}

void DFRobotDFPlayerMini::disableLoopAll(){
  sendStack(0x11, 0x00);
}

void DFRobotDFPlayerMini::playMp3Folder(int fileNumber){
  sendStack(0x12, fileNumber);
}

void DFRobotDFPlayerMini::advertise(int fileNumber){
  sendStack(0x13, fileNumber);
}

void DFRobotDFPlayerMini::playLargeFolder(uint8_t folderNumber, uint16_t fileNumber){
  sendStack(0x14, (((uint16_t)folderNumber) << 12) | fileNumber);
}

void DFRobotDFPlayerMini::stopAdvertise(){
  sendStack(0x15);
}

void DFRobotDFPlayerMini::stop(){
  sendStack(0x16);
}

void DFRobotDFPlayerMini::loopFolder(int folderNumber){
  sendStack(0x17, folderNumber);
}

void DFRobotDFPlayerMini::randomAll(){
  sendStack(0x18);
}

void DFRobotDFPlayerMini::enableLoop(){
  sendStack(0x19, 0x00);
}

void DFRobotDFPlayerMini::disableLoop(){
  sendStack(0x19, 0x01);
}

void DFRobotDFPlayerMini::enableDAC(){
  sendStack(0x1A, 0x00);
}

void DFRobotDFPlayerMini::disableDAC(){
  sendStack(0x1A, 0x01);
}

int DFRobotDFPlayerMini::readState(){
  sendStack(0x42);
  if (waitAvailable()) {
    return read();
  }
  else{
    return -1;
  }
}

int DFRobotDFPlayerMini::readVolume(){
  sendStack(0x43);
  if (waitAvailable()) {
    return read();
  }
  else{
    return -1;
  }
}

uint8_t DFRobotDFPlayerMini::readEQ(){
  sendStack(0x44);
  if (waitAvailable()) {
    return read();
  }
  else{
    return -1;
  }
}

int DFRobotDFPlayerMini::readFileCounts(uint8_t device){
  switch (device) {
    case DFPLAYER_DEVICE_U_DISK:
      sendStack(0x47);
      break;
    case DFPLAYER_DEVICE_SD:
      sendStack(0x48);
      break;
    case DFPLAYER_DEVICE_FLASH:
      sendStack(0x49);
      break;
    default:
      break;
  }
  
  if (waitAvailable()) {
    return read();
  }
  else{
    return -1;
  }
}

int DFRobotDFPlayerMini::readCurrentFileNumber(uint8_t device){
  switch (device) {
    case DFPLAYER_DEVICE_U_DISK:
      sendStack(0x4B);
      break;
    case DFPLAYER_DEVICE_SD:
      sendStack(0x4C);
      break;
    case DFPLAYER_DEVICE_FLASH:
      sendStack(0x4D);
      break;
    default:
      break;
  }
  if (waitAvailable()) {
    return read();
  }
  else{
    return -1;
  }
}

int DFRobotDFPlayerMini::readFileCountsInFolder(int folderNumber){
  sendStack(0x4E, folderNumber);
  if (waitAvailable()) {
    return read();
  }
  else{
    return -1;
  }
}

int DFRobotDFPlayerMini::readFolderCounts(){
  sendStack(0x4F);
  if (waitAvailable()) {
    return read();
  }
  else{
    return -1;
  }
}

int DFRobotDFPlayerMini::readFileCounts(){
  return readFileCounts(DFPLAYER_DEVICE_SD);
}

int DFRobotDFPlayerMini::readCurrentFileNumber(){
  return readCurrentFileNumber(DFPLAYER_DEVICE_SD);
}


