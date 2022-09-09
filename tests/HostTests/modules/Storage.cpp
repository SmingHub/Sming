#include <HostTests.h>
#include <Storage.h>
#include <Storage/Debug.h>

class TestDevice : public Storage::Device
{
public:
	String getName() const override
	{
		return F("testDevice");
	}

	size_t getBlockSize() const override
	{
		return sizeof(uint32_t);
	}

	size_t getSize() const override
	{
		return 0x40000000;
	}

	Type getType() const override
	{
		return Type::unknown;
	}

	bool read(uint32_t address, void* dst, size_t size) override
	{
		for(unsigned i = 0; i < size; ++i) {
			static_cast<uint8_t*>(dst)[i] = address + i;
		}
		return true;
	}

	bool write(uint32_t address, const void* src, size_t size) override
	{
		return false;
	}

	bool erase_range(uint32_t address, size_t size) override
	{
		return false;
	}
};

class PartitionTest : public TestGroup
{
public:
	PartitionTest() : TestGroup(_F("Partition"))
	{
	}

	void execute() override
	{
		auto dev = new TestDevice;
		Storage::registerDevice(dev);

		listPartitions();

		delete dev;
	}

	void listPartitions()
	{
		for(auto part : Storage::findPartition()) {
			Serial << "* " << part << endl;

			testRead(part, 0xE0, 0x20, true);
			testRead(part, 10, 20, true);
			testRead(part, part.size() - 10, 10, true);
			testRead(part, part.size() - 10, 11, false);
		}
	}

	void testRead(Storage::Partition& part, uint32_t address, size_t size, bool shouldSucceed)
	{
		auto buf = new uint8_t[size];
		bool success = part.read(address, buf, size);
		if(success) {
			m_printHex("READ", buf, size, address);
			REQUIRE(shouldSucceed == true);
		} else {
			debug_e("read(0x%08x, %u) failed", address, size);
			REQUIRE(shouldSucceed == false);
		}
		delete[] buf;
	}
};

void REGISTER_TEST(Storage)
{
	registerGroup<PartitionTest>();
}
