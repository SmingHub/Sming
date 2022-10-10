/**
 * Error.cpp
 * SPIFFS error codes
 *
 * Created on: 31 Aug 2018
 *
 * Copyright 2019 mikee47 <mike@sillyhouse.net>
 *
 * This file is part of the SPIFFS IFS Library
 *
 * This library is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, version 3 or later.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this library.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 ****/

#include "include/IFS/SPIFFS/Error.h"
#include <IFS/Error.h>
#include <FlashString/Map.hpp>

namespace IFS
{
namespace SPIFFS
{
/*
 * @todo Return generic FSERR codes wherever possible by mapping from SPIFFS codes
 */

/*
 * Translate common SPIFFS error codes into the standard one.
 * This avoids the need to provide strings for these values.
 */
#define SPIFFS_ERROR_TRANSLATION_MAP(XX)                                                                               \
	XX(SPIFFS_ERR_NOT_MOUNTED, Error::NotMounted)                                                                      \
	XX(SPIFFS_ERR_FULL, Error::NoSpace)                                                                                \
	XX(SPIFFS_ERR_NOT_FOUND, Error::NotFound)                                                                          \
	XX(SPIFFS_ERR_END_OF_OBJECT, Error::SeekBounds)                                                                    \
	XX(SPIFFS_ERR_DELETED, Error::InvalidHandle)                                                                       \
	XX(SPIFFS_ERR_FILE_CLOSED, Error::FileNotOpen)                                                                     \
	XX(SPIFFS_ERR_OUT_OF_FILE_DESCS, Error::OutOfFileDescs)                                                            \
	XX(SPIFFS_ERR_BAD_DESCRIPTOR, Error::InvalidHandle)                                                                \
	XX(SPIFFS_ERR_NOT_WRITABLE, Error::ReadOnly)                                                                       \
	XX(SPIFFS_ERR_NOT_READABLE, Error::Denied)                                                                         \
	XX(SPIFFS_ERR_CONFLICTING_NAME, Error::Exists)                                                                     \
	XX(SPIFFS_ERR_NOT_CONFIGURED, Error::BadFileSystem)                                                                \
	XX(SPIFFS_ERR_NOT_A_FS, Error::BadFileSystem)                                                                      \
	XX(SPIFFS_ERR_MOUNTED, Error::Denied)                                                                              \
	XX(SPIFFS_ERR_ERASE_FAIL, Error::EraseFailure)                                                                     \
	XX(SPIFFS_ERR_FILE_EXISTS, Error::Exists)                                                                          \
	XX(SPIFFS_ERR_RO_NOT_IMPL, Error::ReadOnly)                                                                        \
	XX(SPIFFS_ERR_RO_ABORTED_OPERATION, Error::ReadOnly)                                                               \
	XX(SPIFFS_ERR_PROBE_NOT_A_FS, Error::BadFileSystem)                                                                \
	XX(SPIFFS_ERR_NAME_TOO_LONG, Error::NameTooLong)                                                                   \
	XX(SPIFFS_ERR_SEEK_BOUNDS, Error::SeekBounds)

/*
 * All remaining SPIFFS error codes
 */
#define SPIFFS_ERROR_MAP(XX)                                                                                           \
	XX(NOT_FINALIZED, -10005)                                                                                          \
	XX(NOT_INDEX, -10006)                                                                                              \
	XX(OUT_OF_FILE_DESCS, -10007)                                                                                      \
	XX(FILE_DELETED, -10009)                                                                                           \
	XX(IS_INDEX, -10011)                                                                                               \
	XX(IS_FREE, -10012)                                                                                                \
	XX(INDEX_SPAN_MISMATCH, -10013)                                                                                    \
	XX(DATA_SPAN_MISMATCH, -10014)                                                                                     \
	XX(INDEX_REF_FREE, -10015)                                                                                         \
	XX(INDEX_REF_LU, -10016)                                                                                           \
	XX(INDEX_REF_INVALID, -10017)                                                                                      \
	XX(INDEX_FREE, -10018)                                                                                             \
	XX(INDEX_LU, -10019)                                                                                               \
	XX(INDEX_INVALID, -10020)                                                                                          \
	XX(MAGIC_NOT_POSSIBLE, -10028)                                                                                     \
	XX(NO_DELETED_BLOCKS, -10029)                                                                                      \
	XX(NOT_A_FILE, -10031)                                                                                             \
	XX(PROBE_TOO_FEW_BLOCKS, -10034)                                                                                   \
	XX(IX_MAP_UNMAPPED, -10037)                                                                                        \
	XX(IX_MAP_MAPPED, -10038)                                                                                          \
	XX(IX_MAP_BAD_RANGE, -10039)                                                                                       \
	XX(INTERNAL, -10050)                                                                                               \
	XX(TEST, -10100)

#define XX(tag, value) DEFINE_FSTR_LOCAL(str_##tag, #tag)
SPIFFS_ERROR_MAP(XX)
#undef XX

#define XX(tag, value) {value, &str_##tag},
DEFINE_FSTR_MAP_LOCAL(errorMap, int, FlashString, SPIFFS_ERROR_MAP(XX))
#undef XX

int translateSpiffsError(int spiffs_error)
{
	switch(spiffs_error) {
#define XX(err_spiffs, err_sys)                                                                                        \
	case err_spiffs:                                                                                                   \
		return err_sys;
	default:
		return Error::fromSystem(spiffs_error);
	}
}

String spiffsErrorToStr(int err)
{
	return errorMap[std::min(err, 0)].content();
}

} // namespace SPIFFS
} // namespace IFS
