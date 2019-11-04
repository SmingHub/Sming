#include "include/RttlWriter.h"
#include <muldiv.h>

namespace RingTone
{
void RtttlWriter::beginTune(const RtttlHeader& header)
{
	out.print(header.title);
	out.print(':');
	out.print("d=");
	out.print(header.defaultNoteDuration);
	out.print(",o=");
	out.print(header.defaultOctave);
	out.print(",b=");
	out.print(header.bpm);
	out.print(':');
	++tuneCount;
	noteCount = 0;
	this->header = header;
}

void RtttlWriter::addNote(const NoteDef& note)
{
	if(noteCount++ > 0) {
		out.print(',');
	}

	// Duration, express as fraction of beat
	bool dotted = false;
	unsigned frac = muldiv(4U * 60U * 1000U, 1U, unsigned(note.duration) * header.bpm);
	if(frac != header.defaultNoteDuration) {
		switch(frac) {
		case 1:
		case 2:
		case 4:
		case 8:
		case 16:
		case 32:
		case 64:
			dotted = false;
			break;
		default:
			dotted = (frac % 2 == 0);
		}
		if(dotted) {
			frac = frac * 3 / 2;
		}
		if(frac != header.defaultNoteDuration) {
			out.print(frac);
		}
	}

	// Note
	unsigned octave;
	unsigned noteValue = RingTone::getClosestNote(note.frequency, octave);
	out.print(RingTone::getNoteName(noteValue));

	// Optional '.' dotted note
	if(dotted) {
		out.print('.');
	}

	// Scale
	if(noteValue != 0 && octave != header.defaultOctave) {
		out.print(octave);
	}
}

void RtttlWriter::endTune()
{
	out.println();
	noteCount = 0;
}

void RtttlWriter::addComment(const String& text)
{
	if(noteCount > 0) {
		out.println();
	}
	out.print("# ");
	out.println(text);
	if(noteCount > 0) {
		out.print('\t');
	}
}

void RtttlWriter::addTune(RtttlParser& parser)
{
	beginTune(parser.getHeader());

	NoteDef note;
	while(parser.readNextNote(note)) {
		addNote(note);
	}
	endTune();
}

void RtttlWriter::addAllTunes(RtttlParser& parser)
{
	do {
		addTune(parser);
	} while(parser.nextTune());
}

} // namespace RingTone
