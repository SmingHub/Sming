/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Directory.h
 *
 * @author mikee47 <mike@sillyhouse.net> May 2019
 *
 ****/

#pragma once

#include "FsBase.h"

namespace IFS
{
/**
  * @brief Wrapper class for enumerating a directory
 */
class Directory : public FsBase
{
public:
	using FsBase::FsBase;

	~Directory()
	{
		close();
	}

	/**
	 * @brief Open a directory and attach this stream object to it
	 * @param dirName
	 * @retval bool true on success, false on error
	 * @note call getLastError() to determine cause of failure
	 */
	bool open(const String& dirName);

	/**
	 * @brief Close directory
	 */
	void close();

	/**
	 * @brief Rewind directory stream to start so it can be re-enumerated
	 * @retval bool true on success, false on error
	 * @note call getLastError() to determine cause of failure
	 */
	bool rewind();

	/**
	 * @brief Name of directory stream is attached to
	 * @retval String invalid if stream isn't open
	 */
	const String& getDirName() const
	{
		return name;
	}

	/**
	 * @brief Determine if directory exists
	 * @retval bool true if stream is attached to a directory
	 */
	bool dirExist() const
	{
		return dir != nullptr;
	}

	/**
	 * @brief Get path with leading separator /path/to/dir
	 */
	String getPath() const;

	/**
	 * @brief Get parent directory
	 * @retval String invalid if there is no parent directory
	 */
	String getParent() const;

	int index() const
	{
		return currentIndex;
	}

	bool isValid() const
	{
		return currentIndex >= 0;
	}

	size_t size() const
	{
		return totalSize;
	}

	const FileStat& stat() const
	{
		return dirStat;
	}

	bool next();

private:
	String name;
	DirHandle dir{};
	FileNameStat dirStat;
	int currentIndex{-1};
	size_t totalSize{0};
};

} // namespace IFS
