/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SectionStream.cpp
 *
 * @author mikee47 <mike@sillyhouse.net> Nov 2020
 *
 ****/

#include "SectionStream.h"
#include <debug_progmem.h>

/*
 * Scan through entire source stream to map location and size of sections.
 * Called once by constructor.
 */
void SectionStream::scanSource(uint8_t maxSections)
{
	constexpr size_t bufSize{512};
	char buffer[bufSize];

	sections.reset(new Section[maxSections]{});

	size_t offset{0};
	while(sectionCount < maxSections && !stream->isFinished()) {
		auto& sect = sections[sectionCount];
		stream->seekFrom(offset, SeekOrigin::Start);
		size_t bytesRead = stream->readMemoryBlock(buffer, bufSize);
		if(sect.start == 0) {
			auto tag = (char*)memmem(buffer, bytesRead, startTag.c_str(), startTag.length());
			if(tag == nullptr) {
				if(bytesRead < bufSize) {
					break;
				}
				// If tag starts near end of buffer, this ensures it'll be contained in next read
				offset += bytesRead - startTag.length();
				continue;
			}

			offset += (tag - buffer) + startTag.length();
			sect.start = offset;
			continue;
		}

		auto tag = (char*)memmem(buffer, bytesRead, endTag.c_str(), endTag.length());
		if(tag == nullptr) {
			if(bytesRead < bufSize) {
				break;
			}
			offset += bytesRead - endTag.length();
			continue;
		}
		offset += (tag - buffer);
		sect.size = offset - sect.start;
		offset += endTag.length();
		++sectionCount;
	}

	// No sections? Then consider the entire stream as a single section
	if(sectionCount == 0) {
		sections[0].start = 0;
		sections[0].size = stream->seekFrom(0, SeekOrigin::End);
		sectionCount = 1;
	}

	stream->seekFrom(0, SeekOrigin::Start);
}

bool SectionStream::gotoSection(uint8_t index)
{
	if(index >= sectionCount) {
		debug_e("gotoSection: %u out of range", index);
		return false;
	}

	auto& sect = sections[index];

	if(stream->seekFrom(sect.start, SeekOrigin::Start) != int(sect.start)) {
		return false;
	}

	readOffset = sect.start;
	sectionOffset = 0;
	sect.recordIndex = -1;
	currentSectionIndex = int(index) - 1;
	finished = false;
	sections[index].recordCount = 0;

	nextSection();

	return true;
}

void SectionStream::nextSection()
{
	do {
		++currentSectionIndex;
		if(currentSectionIndex >= sectionCount) {
			finished = true;
			return;
		}
		auto& sect = sections[currentSectionIndex];
		sectionOffset = 0;
		sect.recordCount = 0; // Wipe any existing count
		sect.recordIndex = -1;
		readOffset = stream->seekFrom(sect.start, SeekOrigin::Start);
		if(readOffset != sect.start) {
			debug_e("SectionStream: Failed to seek source");
			finished = true;
			return;
		}

		if(nextSectionCallback) {
			nextSectionCallback();
		}

	} while(!nextRecord());

	sections[currentSectionIndex].recordIndex = 0;
}

uint16_t SectionStream::readMemoryBlock(char* data, int bufSize)
{
	if(currentSectionIndex < 0) {
		nextSection();
	}

	if(finished || currentSectionIndex < 0) {
		return 0;
	}

	auto sect = &sections[currentSectionIndex];
	if(sectionOffset >= sect->size) {
		if(nextRecord()) {
			++sect->recordIndex;
			++sect->recordCount;
			readOffset = stream->seekFrom(sect->start, SeekOrigin::Start);
			sectionOffset = 0;
		} else {
			nextSection();
			if(finished) {
				return 0;
			}
			sect = &sections[currentSectionIndex];
		}
	}

	bufSize = std::min(uint32_t(bufSize), sect->size - sectionOffset);

	return stream->readMemoryBlock(data, bufSize);
}

int SectionStream::seekFrom(int offset, SeekOrigin origin)
{
	uint32_t newOffset = readOffset;

	if(currentSectionIndex < 0 || currentSectionIndex >= sectionCount) {
		return -1;
	}
	auto& sect = sections[currentSectionIndex];

	switch(origin) {
	case SeekOrigin::Start:
		newOffset = sect.start + offset;
		break;

	case SeekOrigin::Current:
		newOffset += offset;
		break;

	case SeekOrigin::End:
		newOffset = uint32_t(sect.end() + offset);
		break;

	default:
		return -1;
	}

	if(newOffset < sect.start || newOffset > sect.end()) {
		return -1;
	}

	int pos = stream->seekFrom(newOffset, SeekOrigin::Start);
	if(uint32_t(pos) != newOffset) {
		return -1;
	}

	readOffset = newOffset;
	sectionOffset = newOffset - sect.start;

	if(newSection >= 0) {
		// Prevent recursion
		auto n = newSection;
		newSection = -1;
		gotoSection(n);
	}

	return sectionOffset;
}
