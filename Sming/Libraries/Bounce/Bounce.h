
/*
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *      
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *      
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */



/*  * * * * * * * * * * * * * * * * * * * * * * * * * * * 
 Main code by Thomas O Fredericks
 Rebounce and duration functions contributed by Eric Lowry
 Write function contributed by Jim Schimpf
 risingEdge and fallingEdge contributed by Tom Harkaway
* * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef Bounce_h
#define Bounce_h

#include <inttypes.h>

class Bounce
{

public:
	// Initialize
  Bounce(uint8_t pin, unsigned long interval_millis ); 
	// Sets the debounce interval
  void interval(unsigned long interval_millis); 
	// Updates the pin
	// Returns 1 if the state changed
	// Returns 0 if the state did not change
  int update(); 
	// Forces the pin to signal a change (through update()) in X milliseconds 
	// even if the state does not actually change
	// Example: press and hold a button and have it repeat every X milliseconds
  void rebounce(unsigned long interval); 
	// Returns the updated pin state
  int read();
	// Sets the stored pin state
  void write(int new_state);
    // Returns the number of milliseconds the pin has been in the current state
  unsigned long duration();
  // The risingEdge method is true for one scan after the de-bounced input goes from off-to-on.
	bool risingEdge();
  // The fallingEdge  method it true for one scan after the de-bounced input goes from on-to-off. 
	bool fallingEdge();
  
protected:
  int debounce();
  unsigned long  previous_millis, interval_millis, rebounce_millis;
  uint8_t state;
  uint8_t pin;
  uint8_t stateChanged;
};

#endif


