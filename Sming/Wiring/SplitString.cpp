#include "SplitString.h"

int splitString(String& what, int delim, Vector<long>& splits)
{
	what.trim();
	splits.removeAllElements();
	const char* chars = what.c_str();
	int splitCount = 0; //1;
	for (unsigned i = 0; i < what.length(); i++) {
		if (chars[i] == delim)
			splitCount++;
	}
	if (splitCount == 0) {
		splits.addElement(atol(what.c_str()));
		return 1;
	}

	int pieceCount = splitCount + 1;

	int splitIndex = 0;
	int startIndex = 0;
	for (unsigned i = 0; i < what.length(); i++) {
		if (chars[i] == delim) {
			splits.addElement(atol(what.substring(startIndex, i).c_str()));
			splitIndex++;
			startIndex = i + 1;
		}
	}
	splits.addElement(atol(what.substring(startIndex, what.length()).c_str()));

	return pieceCount;
}

int splitString(String& what, int delim, Vector<int>& splits)
{
	what.trim();
	splits.removeAllElements();
	const char* chars = what.c_str();
	int splitCount = 0; //1;
	for (unsigned i = 0; i < what.length(); i++) {
		if (chars[i] == delim)
			splitCount++;
	}
	if (splitCount == 0) {
		splits.addElement(atoi(what.c_str()));
		return (1);
	}

	int pieceCount = splitCount + 1;

	int splitIndex = 0;
	int startIndex = 0;
	for (unsigned i = 0; i < what.length(); i++) {
		if (chars[i] == delim) {
			splits.addElement(atoi(what.substring(startIndex, i).c_str()));
			splitIndex++;
			startIndex = i + 1;
		}
	}
	splits.addElement(atoi(what.substring(startIndex, what.length()).c_str()));

	return pieceCount;
}

int splitString(String& what, int delim, Vector<String>& splits)
{
	what.trim();
	splits.removeAllElements();
	const char* chars = what.c_str();
	int splitCount = 0;

	int splitIndex = 0;
	int startIndex = 0;
	for (unsigned i = 0; i < what.length(); i++) {
		if (chars[i] == delim) {
			splits.addElement(what.substring(startIndex, i));
			splitIndex++;
			startIndex = i + 1;
			splitCount++;
		}
	}
	splits.addElement(what.substring(startIndex, what.length()));

	return (splitCount + 1);
}
