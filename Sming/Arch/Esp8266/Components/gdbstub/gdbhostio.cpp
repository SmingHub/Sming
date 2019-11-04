/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * gdbhostio.h
 *
 * @author: 2019 - Mikee47 <mike@sillyhouse.net>
 *
 * Deals with Host I/O commands from GDB
 *
 ****/

#include "gdbstub-cfg.h"

#if GDBSTUB_ENABLE_HOSTIO

#include "GdbPacket.h"
#include <sys/errno.h>
#include <fcntl.h>
#include "FileSystem.h"
#include "WString.h"

void ATTR_GDBEXTERNFN gdbHandleHostIo(char* commandBuffer, unsigned cmdLen)
{
	const char* data = &commandBuffer[1];

	auto match = [&](const char* tag) -> bool {
		auto len = strlen(tag);
		if(data[len] == ':' && memcmp(data, tag, len) == 0) {
			data += len + 1;
			return true;
		} else {
			return false;
		}
	};

	// Start a packet for the result
	GdbPacket packet;

	/*
	 * Host I/O
	 *
	 * `vFile:operation:parameter...`
	 */
	if(!match(GDB_F("File"))) {
		return; // empty packet response
	}

	/*
	 * open: filename, flags, mode
	 */
	if(match(GDB_F("open"))) {
		char* filename = commandBuffer;
		size_t len = GdbPacket::decodeHexBlock(filename, data);
		filename[len] = '\0';
		++data; // skip ,
		unsigned flags = GdbPacket::readHexValue(data);
		++data;						   // Skip ,
		GdbPacket::readHexValue(data); // Skip mode (not used)

		FileOpenFlags openFlags;
		if((flags & 0xff) == O_RDWR) {
			openFlags = eFO_ReadWrite;
		} else if((flags & 0xff) == O_WRONLY) {
			openFlags = eFO_WriteOnly;
		} else {
			openFlags = eFO_ReadOnly;
		}
		if(flags & O_CREAT) {
			openFlags = openFlags | eFO_CreateIfNotExist;
		}
		if(flags & O_TRUNC) {
			openFlags = openFlags | eFO_Truncate;
		}
		int fd = fileOpen(filename, openFlags);

		debug_i("File:open('%s', 0x%04x, 0x%04x), %d", filename, flags, mode, fd);

		packet.writeChar('F');
		if(fd < 0) {
			packet.writeStr(_F("-1,"));
			packet.writeHexByte(ENOENT);
		} else {
			packet.writeHexByte(fd);
		}
		return;
	}

	/*
	 * close: fd, count, offset
	 */
	if(match(GDB_F("close"))) {
		int fd = GdbPacket::readHexValue(data);
		debug_i("File:close(%d)", fd);
		fileClose(fd);

		packet.writeChar('F');
		packet.writeChar('0');
		return;
	}

	/*
	 * pread: fd, count, offset
	 */
	if(match(GDB_F("pread"))) {
		int fd = GdbPacket::readHexValue(data);
		++data; // Skip ,
		unsigned count = GdbPacket::readHexValue(data);
		++data; // Skip ,
		unsigned offset = GdbPacket::readHexValue(data);
		debug_i("File:pread(%d, %u, %u)", fd, count, offset);

		packet.writeChar('F');
		if(fileSeek(fd, offset, eSO_FileStart) == int(offset)) {
			count = fileRead(fd, commandBuffer, count);
			if(int(count) >= 0) {
				packet.writeHexWord16(count);
				packet.writeChar(';');
				packet.writeEscaped(commandBuffer, count);
				return;
			}
		}
		packet.writeStr(_F("-1,"));
		packet.writeHexByte(EIO);
		return;
	}

	/*
	 * pwrite: fd, offset, data
	 */
	if(match(GDB_F("pwrite"))) {
		int fd = GdbPacket::readHexValue(data);
		++data; // Skip ,
		unsigned offset = GdbPacket::readHexValue(data);
		++data; // Skip ,
		unsigned size = cmdLen - (data - commandBuffer);
		debug_i("File:pwrite(%d, %u, %u)", fd, offset, size);

		packet.writeChar('F');
		if(fileSeek(fd, offset, eSO_FileStart) == int(offset)) {
			int count = fileWrite(fd, data, size);
			if(count >= 0) {
				packet.writeHexWord16(count);
				return;
			}
		}
		packet.writeStr(_F("-1,"));
		packet.writeHexByte(EIO);
	}

	/*
	 * unlink: filename
	 */
	if(match(GDB_F("unlink"))) {
		char* filename = commandBuffer;
		size_t len = GdbPacket::decodeHexBlock(filename, data);
		filename[len] = '\0';
		int res = fileDelete(filename);
		debug_i("File:delete('%s'), %d", filename, res);

		packet.writeChar('F');
		if(res < 0) {
			packet.writeStr(_F("-1,"));
			packet.writeHexByte(ENOENT);
		} else {
			packet.writeHexByte(0);
		}
		return;
	}
}

#endif // GDBSTUB_ENABLE_HOSTIO
