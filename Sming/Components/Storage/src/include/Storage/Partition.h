/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Partition.h - C++ wrapper for universal partition table support
 *
 * Original license for IDF code:
 *
 *	Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
 *
 *	Licensed under the Apache License, Version 2.0 (the "License");
 *	you may not use this file except in compliance with the License.
 *	You may obtain a copy of the License at
 *
 *	 http://www.apache.org/licenses/LICENSE-2.0
 *
 *	Unless required by applicable law or agreed to in writing, software
 *	distributed under the License is distributed on an "AS IS" BASIS,
 *	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	See the License for the specific language governing permissions and
 *	limitations under the License.
 *
 *
 ****/
#pragma once

#include <Data/BitSet.h>
#include <Data/CString.h>
#include <memory>
#include <cassert>

#define PARTITION_APP_SUBTYPE_MAP(XX)                                                                                  \
	XX(factory, 0x00, "Factory application")                                                                           \
	XX(ota0, 0x10, "OTA #0")                                                                                           \
	XX(ota1, 0x11, "OTA #1")                                                                                           \
	XX(ota2, 0x12, "OTA #2")                                                                                           \
	XX(ota3, 0x13, "OTA #3")                                                                                           \
	XX(ota4, 0x14, "OTA #4")                                                                                           \
	XX(ota5, 0x15, "OTA #5")                                                                                           \
	XX(ota6, 0x16, "OTA #6")                                                                                           \
	XX(ota7, 0x17, "OTA #7")                                                                                           \
	XX(ota8, 0x18, "OTA #8")                                                                                           \
	XX(ota9, 0x19, "OTA #9")                                                                                           \
	XX(ota10, 0x1a, "OTA #10")                                                                                         \
	XX(ota11, 0x1b, "OTA #11")                                                                                         \
	XX(ota12, 0x1c, "OTA #12")                                                                                         \
	XX(ota13, 0x1d, "OTA #13")                                                                                         \
	XX(ota14, 0x1e, "OTA #14")                                                                                         \
	XX(ota15, 0x1f, "OTA #15")                                                                                         \
	XX(test, 0x20, "Test application")

#define PARTITION_DATA_SUBTYPE_MAP(XX)                                                                                 \
	XX(ota, 0x00, "OTA selection")                                                                                     \
	XX(phy, 0x01, "PHY init data")                                                                                     \
	XX(nvs, 0x02, "NVS")                                                                                               \
	XX(coreDump, 0x03, "Core Dump data")                                                                               \
	XX(nvsKeys, 0x04, "NVS key information")                                                                           \
	XX(eFuseEm, 0x05, "eFuse emulation")                                                                               \
	XX(sysParam, 0x40, "System Parameters")                                                                            \
	XX(rfCal, 0x41, "RF Calibration")                                                                                  \
	XX(espHttpd, 0x80, "ESPHTTPD")                                                                                     \
	XX(fat, 0x81, "FAT")                                                                                               \
	XX(spiffs, 0x82, "SPIFFS")                                                                                         \
	XX(fwfs, 0xF1, "FWFS")                                                                                             \
	XX(littlefs, 0xF2, "LittleFS")

namespace Storage
{
class Device;
class PartitionTable;
struct esp_partition_info_t;

/**
 * @brief Represents a flash partition
 */
class Partition
{
public:
	enum class Type : uint8_t {
		app = 0x00,
		data = 0x01,
		storage = 0x02,
		userMin = 0x40,
		userMax = 0xFE,
		invalid = 0xff,
		any = 0xff,
	};

	struct SubType {
		static constexpr uint8_t any{0xff};
		static constexpr uint8_t invalid{0xff};

		/**
		 * @brief Application partition type
		 */
		enum class App : uint8_t {
			partitionType = uint8_t(Type::app),
#define XX(type, value, desc) type = value,
			PARTITION_APP_SUBTYPE_MAP(XX)
#undef XX
				ota_min = ota0,
			ota_max = ota15,
			any = 0xff
		};

		/**
		 * @brief Data partition type
		 */
		enum class Data : uint8_t {
			partitionType = uint8_t(Type::data),
#define XX(subtype, value, desc) subtype = value,
			PARTITION_DATA_SUBTYPE_MAP(XX)
#undef XX
				any = 0xff
		};
	};

	enum class Flag {
		encrypted = 0,
		readOnly = 31, ///< Write/erase prohibited
	};

	static constexpr size_t nameSize{16};
	using Name = char[nameSize];
	using Flags = BitSet<uint32_t, Flag>;

	/**
	 * @brief Partition information
	 */
	struct Info {
		CString name;
		uint32_t offset{0};
		uint32_t size{0};
		Type type{Type::invalid};
		uint8_t subtype{SubType::invalid};
		Flags flags;

		Info()
		{
		}

		Info(const String& name, Type type, uint8_t subtype, uint32_t offset, uint32_t size, Flags flags)
			: name(name), offset(offset), size(size), type(type), subtype(subtype), flags(flags)
		{
		}
	};

	Partition()
	{
	}

	Partition(const Partition& other) : mDevice(other.mDevice), mPart(other.mPart)
	{
	}

	Partition(Device& device, const Info& info) : mDevice(&device), mPart(&info)
	{
	}

	/**
	 * @name Confirm partition is of the expected type
	 * @{
	 */

	/**
	 * @brief Strong C++ type value
	 * @param type Expected partition type
	 * @param subtype Expected partition sub-type
	 * @retval bool true if type is OK, false if not.
	 * Logs debug messages on failure.
	 */
	bool verify(Type type, uint8_t subtype) const;

	/// Weak 'type' value
	bool verify(uint8_t type, uint8_t subtype) const
	{
		return verify(Type(type), subtype);
	}

	/// Derive type from subtype, expressed as strong C++ enum
	template <typename T> bool verify(T subType) const
	{
		return verify(Type(T::partitionType), uint8_t(subType));
	}

	/** @} */

	/**
	 * @brief Convenience function to get SubType value for the i-th OTA partition
	 */
	static inline SubType::App apptypeOta(uint8_t i)
	{
		auto subtype = SubType::App(uint8_t(SubType::App::ota_min) + i);
		assert(subtype >= SubType::App::ota_min && subtype <= SubType::App::ota_max);
		return subtype;
	}

	explicit operator bool() const
	{
		return mDevice != nullptr && mPart != nullptr;
	}

	/**
	 * @brief Read data from the partition
	 * @param offset Where to start reading, relative to start of partition
	 * @param dst Buffer to store data
	 * @param size Size of data to be read, in bytes.
	 * @retval bool true on success, false on error
	 */
	bool read(size_t offset, void* dst, size_t size);

	template <typename T> typename std::enable_if<std::is_pod<T>::value, bool>::type read(size_t offset, T& value)
	{
		return read(offset, &value, sizeof(value));
	}

	/**
	 * @brief Write data to the partition
	 * @param offset Where to start writing, relative to start of partition
	 * @param src Data to write
	 * @param size Size of data to be written, in bytes.
	 * @retval bool true on success, false on error
	 * @note Flash region must be erased first
	 */
	bool write(size_t offset, const void* src, size_t size);

	/**
	 * @brief Erase part of the partition
	 * @param offset Where to start erasing, relative to start of partition
	 * @param size Size of region to erase, in bytes
	 * @retval bool true on success, false on error
	 * @note Both offset and size must be aligned to flash sector size (4Kbytes)
	 */
	bool erase_range(size_t offset, size_t size);

	/**
	 * @brief Obtain partition type
	 */
	Partition::Type type() const
	{
		return mPart ? Partition::Type(mPart->type) : Type::invalid;
	}

	/**
	 * @brief Obtain partition sub-type
	 */
	uint8_t subType() const
	{
		return mPart ? mPart->subtype : SubType::invalid;
	}

	/**
	 * @brief Obtain partition starting address
	 * @param uint32_t Device address
	 */
	uint32_t address() const
	{
		return (mPart && mPart->type != Partition::Type::storage) ? mPart->offset : 0;
	}

	/**
	 * @brief Obtain address of last byte in this this partition
	 * @param uint32_t Device address
	 */
	uint32_t lastAddress() const
	{
		return mPart ? (mPart->offset + mPart->size - 1) : 0;
	}

	/**
	 * @brief Obtain partition size
	 * @retval size_t Size in bytes
	 */
	size_t size() const
	{
		return mPart ? mPart->size : 0;
	}

	/**
	 * @brief Get partition name
	 */
	String name() const
	{
		return mPart ? mPart->name.c_str() : nullptr;
	}

	/**
	 * @brief Get partition flags
	 */
	Flags flags() const
	{
		return mPart ? mPart->flags : 0;
	}

	/**
	 * @brief Check state of partition `encrypted` flag
	 */
	bool isEncrypted() const
	{
		return flags()[Flag::encrypted];
	}

	/**
	 * @brief Check state of partition `readOnly` flag
	 */
	bool isReadOnly() const
	{
		return mPart ? mPart->flags[Flag::readOnly] : true;
	}

	/**
	 * @name Get partition type expressed as a string
	 * @{
	 */
	String typeString() const;
	String longTypeString() const;
	/** @} */

	/**
	 * @brief Get corresponding storage device address for a given partition offset
	 * @param address IN: Zero-based offset within partition, OUT: Device address
	 * @param size Size of data to be accessed
	 * @retval bool true on success, false on failure
	 * Fails if the given offset/size combination is out of range, or the partition is undefined.
	 */
	bool getDeviceAddress(uint32_t& address, size_t size) const;

	/**
	 * @brief Get name of storage device for this partition
	 * @retval String
	 */
	String getDeviceName() const;

	/**
	 * @brief Get storage device containing this partition
	 * @retval Device* null if device isn't registered
	 */
	Device* getDevice() const
	{
		return mDevice;
	}

	/**
	 * @brief Determine if given address contained within this partition
	 */
	bool contains(uint32_t addr) const
	{
		return mPart ? (addr >= mPart->offset && addr <= lastAddress()) : false;
	}

	bool operator==(const Partition& other) const
	{
		return this == &other;
	}

	bool operator==(const char* name) const
	{
		return mPart ? mPart->name.equals(name) : false;
	}

	bool operator==(const String& name) const
	{
		return mPart ? mPart->name.equals(name) : false;
	}

	/**
	 * @brief Obtain smallest allocation unit for erase operations
	 */
	size_t getBlockSize() const;

protected:
	Device* mDevice{nullptr};
	const Info* mPart{nullptr};

private:
	bool allowRead();
	bool allowWrite();
};

} // namespace Storage

String toString(Storage::Partition::Type type, uint8_t subType);
String toLongString(Storage::Partition::Type type, uint8_t subType);

template <typename E> typename std::enable_if<bool(E::partitionType), String>::type toString(E subType)
{
	return toString(Storage::Partition::Type(E::partitionType), uint8_t(subType));
}

template <typename E> typename std::enable_if<bool(E::partitionType), String>::type toLongString(E subType)
{
	return toLongString(Storage::Partition::Type(E::partitionType), uint8_t(subType));
}
