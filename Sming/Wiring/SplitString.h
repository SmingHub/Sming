/* $Id: WMemory.cpp 1156 2011-06-07 04:01:16Z bhagman $
||
|| @author         Hernando Barragan <b@wiring.org.co>
|| @url            http://wiring.org.co/
|| @contribution   Brett Hagman <bhagman@wiring.org.co>
|| @contribution   Alexander Brevig <abrevig@wiring.org.co>
||
|| @license Please see cores/Common/License.txt.
||
*/

#ifndef _SPLIT_STRING_H_
#define _SPLIT_STRING_H_

#include "WVector.h"
#include "WString.h"
#include "WiringFrameworkDependencies.h"

/** @brief split a delimited string list of integers into an array
 *  @param what
 *  @param delim
 *  @param splits
 *  @retval number of items returned in splits (same as splits.count())
 *  @note leading/trailing whitespace is removed from 'what' before parsing
 *  example: "   1,2,3,4,5" returns [1, 2, 3, 4, 5]
 */
int splitString(String &what, char delim,  Vector<int> &splits);

/** @brief split a delimited string list into an array
 *  @param what
 *  @param delim
 *  @param splits
 *  @retval number of items returned in splits (same as splits.count())
 *  @note leading/trailing whitespace is removed from 'what' before parsing
 *  example: "   a,b,c,d,e" returns ["a", "b", "c", "d", "e"]
 */
int splitString(String &what, char delim,  Vector<String> &splits);

#endif
