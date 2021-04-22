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
#include <FlashString/Map.hpp>

namespace IFS
{
namespace SPIFFS
{
/*
 * @todo Return generic FSERR codes whereever possible by mapping from SPIFFS codes
 */

#define SPIFFS_ERROR_MAP(XX)                                                                                           \
	XX(OK, 0)                                                                                                          \
	XX(NOT_MOUNTED, -10000)                                                                                            \
	XX(FULL, -10001)                                                                                                   \
	XX(NOT_FOUND, -10002)                                                                                              \
	XX(END_OF_OBJECT, -10003)                                                                                          \
	XX(DELETED, -10004)                                                                                                \
	XX(NOT_FINALIZED, -10005)                                                                                          \
	XX(NOT_INDEX, -10006)                                                                                              \
	XX(OUT_OF_FILE_DESCS, -10007)                                                                                      \
	XX(FILE_CLOSED, -10008)                                                                                            \
	XX(FILE_DELETED, -10009)                                                                                           \
	XX(BAD_DESCRIPTOR, -10010)                                                                                         \
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
	XX(NOT_WRITABLE, -10021)                                                                                           \
	XX(NOT_READABLE, -10022)                                                                                           \
	XX(CONFLICTING_NAME, -10023)                                                                                       \
	XX(NOT_CONFIGURED, -10024)                                                                                         \
                                                                                                                       \
	XX(NOT_A_FS, -10025)                                                                                               \
	XX(MOUNTED, -10026)                                                                                                \
	XX(ERASE_FAIL, -10027)                                                                                             \
	XX(MAGIC_NOT_POSSIBLE, -10028)                                                                                     \
                                                                                                                       \
	XX(NO_DELETED_BLOCKS, -10029)                                                                                      \
                                                                                                                       \
	XX(FILE_EXISTS, -10030)                                                                                            \
                                                                                                                       \
	XX(NOT_A_FILE, -10031)                                                                                             \
	XX(RO_NOT_IMPL, -10032)                                                                                            \
	XX(RO_ABORTED_OPERATION, -10033)                                                                                   \
	XX(PROBE_TOO_FEW_BLOCKS, -10034)                                                                                   \
	XX(PROBE_NOT_A_FS, -10035)                                                                                         \
	XX(NAME_TOO_LONG, -10036)                                                                                          \
                                                                                                                       \
	XX(IX_MAP_UNMAPPED, -10037)                                                                                        \
	XX(IX_MAP_MAPPED, -10038)                                                                                          \
	XX(IX_MAP_BAD_RANGE, -10039)                                                                                       \
                                                                                                                       \
	XX(SEEK_BOUNDS, -10040)                                                                                            \
                                                                                                                       \
	XX(INTERNAL, -10050)                                                                                               \
                                                                                                                       \
	XX(TEST, -10100)

struct spiffs_error_t {
	int32_t err;
	PGM_P tag;
};

#define XX(tag, value) DEFINE_FSTR_LOCAL(str_##tag, #tag)
SPIFFS_ERROR_MAP(XX)
#undef XX

#define XX(tag, value) {value, &str_##tag},
DEFINE_FSTR_MAP_LOCAL(errorMap, int, FlashString, SPIFFS_ERROR_MAP(XX))
#undef XX

String spiffsErrorToStr(int err)
{
	return errorMap[std::min(err, 0)].content();
}

} // namespace SPIFFS
} // namespace IFS
