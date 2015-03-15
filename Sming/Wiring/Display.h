/* $Id: Display.h 1151 2011-06-06 21:13:05Z bhagman $
||
|| @author         Alexander Brevig <abrevig@wiring.org.co>
|| @url            http://wiring.org.co/
|| @contribution   Brett Hagman <bhagman@wiring.org.co>
||
|| @description
|| | Interface for displays.
|| |
|| | Wiring Common API
|| #
||
|| @license Please see cores/Common/License.txt.
||
*/

#ifndef DISPLAY_H
#define DISPLAY_H

#include "Print.h"
#include "WiringFrameworkIncludes.h"

class Display : public Print
{
  public:        
    virtual void clear(void) = 0;
    virtual void home(void) = 0;
    virtual void setCursor(uint8_t, uint8_t) = 0;
    /* rename shift?
    virtual void scrollUp(void) = 0;
    virtual void scrollDown(void) = 0;
    virtual void scrollLeft(void) = 0;
    virtual void scrollRight(void) = 0;
    */
};

#endif
// DISPLAY_H
