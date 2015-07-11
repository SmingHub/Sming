#include "SplitString.h"

int splitString(String &what, int delim,  Vector<long> &splits)
{
  what.trim();
  splits.removeAllElements();
  const char *chars = what.buffer;
  int splitCount = 0; //1;
  for (int i = 0; i < what.length(); i++)
  {
    if (chars[i] == delim) splitCount++;
  }
  if (splitCount == 0)
  {
    splits.addElement(atol(what.buffer));
    return 1;
  }

  int pieceCount = splitCount + 1;

  int splitIndex = 0;
  int startIndex = 0;
  for (int i = 0; i < what.length(); i++)
  {
    if (chars[i] == delim)
    {
      splits.addElement(atol(what.substring(startIndex, i).buffer));
      splitIndex++;
      startIndex = i + 1;
    }
  }
  splits.addElement(atol(what.substring(startIndex, what.length()).buffer));

  return pieceCount;
}

int splitString(String &what, int delim,  Vector<int> &splits)
{
  what.trim();
  splits.removeAllElements();
  const char *chars = what.buffer;
  int splitCount = 0; //1;
  for (int i = 0; i < what.length(); i++)
  {
    if (chars[i] == delim) splitCount++;
  }
  if (splitCount == 0)
  {
    splits.addElement(atoi(what.buffer));
    return(1);
  }

  int pieceCount = splitCount + 1;

  int splitIndex = 0;
  int startIndex = 0;
  for (int i = 0; i < what.length(); i++)
  {
    if (chars[i] == delim)
    {
      splits.addElement(atoi(what.substring(startIndex, i).buffer));
      splitIndex++;
      startIndex = i + 1;
    }
  }
  splits.addElement(atoi(what.substring(startIndex, what.length()).buffer));

  return pieceCount;
}

int splitString(String &what, int delim,  Vector<String> &splits)
{
	what.trim();
	splits.removeAllElements();
	const char *chars = what.buffer;
	int splitCount = 0;

	int splitIndex = 0;
	int startIndex = 0;
	for (int i = 0; i < what.length(); i++)
	{
	  if (chars[i] == delim)
	  {
	    splits.addElement(what.substring(startIndex, i).buffer);
	    splitIndex++;
	    startIndex = i + 1;
	    splitCount++;
	  }
	}
	splits.addElement(what.substring(startIndex, what.length()).buffer);

	return (splitCount + 1);

}
