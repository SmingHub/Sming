/* $Id: Stream.h 1151 2011-06-06 21:13:05Z bhagman $
||
|| @author         Brett Hagman <bhagman@wiring.org.co>
|| @url            http://wiring.org.co/
|| @contribution   Alexander Brevig <abrevig@wiring.org.co>
|| @contribution   David A. Mellis
||
|| @description
|| | Base class for streams.
|| |
|| | Wiring Common API
|| #
||
|| @notes
|| | Originally discussed here:
|| |
|| | http://code.google.com/p/arduino/issues/detail?id=60
|| #
||
|| @license Please see cores/Common/License.txt.
||
*/

#ifndef STREAM_H
#define STREAM_H

#include "Print.h"
#include "WiringFrameworkDependencies.h"

class Stream : public Print
{
  protected:
    unsigned long _timeout;      // number of milliseconds to wait for the next char before aborting timed read
    unsigned long _startMillis;  // used for timeout measurement
    int timedRead();    // private method to read stream with timeout
    int timedPeek();    // private method to peek stream with timeout
    int peekNextDigit(); // returns the next numeric digit in the stream or -1 if timeout
  
  public:
    //virtual ~Stream() {}
    virtual int available() = 0;
    virtual int peek() = 0;
    virtual int read() = 0;
    virtual void flush() = 0;
  
    Stream() {_timeout=1000; _startMillis=0;}
  // parsing methods
  
    void setTimeout(unsigned long timeout);  // sets maximum milliseconds to wait for stream data, default is 1 second
  
    bool find(char *target);   // reads data from the stream until the target string is found
  // returns true if target string is found, false if timed out (see setTimeout)
  
    bool find(char *target, size_t length);   // reads data from the stream until the target string of given length is found
  // returns true if target string is found, false if timed out
  
    bool findUntil(char *target, char *terminator);   // as find but search ends if the terminator string is found
  
    bool findUntil(char *target, size_t targetLen, char *terminate, size_t termLen);   // as above but search ends if the terminate string is found
  
  
    long parseInt(); // returns the first valid (long) integer value from the current position.
    // initial characters that are not digits (or the minus sign) are skipped
    // integer is terminated by the first character that is not a digit.
  
    float parseFloat();               // float version of parseInt
  
    size_t readBytes( char *buffer, size_t length); // read chars from stream into buffer
    // terminates if length characters have been read or timeout (see setTimeout)
    // returns the number of characters placed in the buffer (0 means no valid data found)
  
    size_t readBytesUntil( char terminator, char *buffer, size_t length); // as readBytes with terminator character
    // terminates if length characters have been read, timeout, or if the terminator character  detected
    // returns the number of characters placed in the buffer (0 means no valid data found)
  
    // Wiring String functions to be added here
    String readString();
    String readStringUntil(char terminator);
  
  protected:
    long parseInt(char skipChar); // as above but the given skipChar is ignored
    // as above but the given skipChar is ignored
    // this allows format characters (typically commas) in values to be ignored
  
    float parseFloat(char skipChar);  // as above but the given skipChar is ignored
};

#endif
// STREAM_H
