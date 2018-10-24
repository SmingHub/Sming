/* $Id: SplitString.cpp 1156 2011-06-07 04:01:16Z bhagman $
||
|| @author         Hernando Barragan <b@wiring.org.co>
|| @url            http://wiring.org.co/
|| @contribution   Brett Hagman <bhagman@wiring.org.co>
|| @contribution   Alexander Brevig <abrevig@wiring.org.co>
||
|| @license Please see cores/Common/License.txt.
||
*/

#include "SplitString.h"

int splitString(String &what, char delim,  Vector<int> &splits)
{
  what.trim();
  splits.removeAllElements();
  const char *chars = what.c_str();
  unsigned splitCount = 0;
  for (unsigned i = 0; i < what.length(); i++)
  {
    if (chars[i] == delim) splitCount++;
  }
  if (splitCount == 0)
  {
    splits.addElement(what.toInt());
    return(1);
  }

  int startIndex = 0;
  for (unsigned i = 0; i < what.length(); i++)
  {
    if(what[i] == delim)
    {
      splits.addElement(what.substring(startIndex, i).toInt());
      startIndex = i + 1;
    }
  }
  splits.addElement(what.substring(startIndex).toInt());

  return splits.count();
}

int splitString(String &what, char delim, Vector<String> &splits)
{
	what.trim();
	splits.removeAllElements();
	const char *chars = what.c_str();

	int startIndex = 0;
	for (unsigned i = 0; i < what.length(); i++)
	{
	  if (chars[i] == delim)
	  {
	    splits.addElement(what.substring(startIndex, i));
	    startIndex = i + 1;
	  }
	}
	splits.addElement(what.substring(startIndex));

	return splits.count();
}
