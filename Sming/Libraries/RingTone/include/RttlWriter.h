#pragma once

#include "RtttlParser.h"
#include <Print.h>

namespace RingTone
{
class RtttlWriter
{
public:
	RtttlWriter(Print& out) : out(out)
	{
	}

	void beginTune(const RtttlHeader& header);
	void addNote(const NoteDef& note);
	void endTune();
	void addComment(const String& text);

	void addTune(RtttlParser& parser);
	void addAllTunes(RtttlParser& parser);

private:
	Print& out;
	RtttlHeader header;
	unsigned tuneCount = 0;
	unsigned noteCount = 0;
};

}; // namespace RingTone
