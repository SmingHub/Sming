#include <HostTests.h>
#include <Storage.h>

class SpiffsTest : public TestGroup
{
public:
	SpiffsTest() : TestGroup(_F("SPIFFS"))
	{
	}

	void execute() override
	{
		TEST_CASE("Cycle flash")
		{
			cycleFlash();
		}
	}

	/*
	 * Repeatedly write to a SPIFFS file until sector #0 gets erased.
	 * At that point, re-mount the filesystem then check the test file is still present.
	 * This confirms that a re-format was not done.
	 */
	void cycleFlash()
	{
		spiffs_unmount();
		if(!spiffs_mount()) {
			debug_e("SPIFFS mount failed");
			return;
		}

		DEFINE_FSTR_LOCAL(testFile, "testfile");
		DEFINE_FSTR_LOCAL(testContent, "Some test content to write to a file");

		auto part = *Storage::findPartition(Storage::Partition::SubType::Data::spiffs);

		// Write to filesystem until sector #0 gets erased
		unsigned writeCount = 0;
		uint32_t word0 = 0;
		do {
			if(fileSetContent(testFile, testContent) != int(testContent.length())) {
				debug_e("fileSetContent() failed");
				TEST_ASSERT(false);
				break;
			}
			++writeCount;
			if(!part.read(0, word0)) {
				debug_e("part.read() failed");
				TEST_ASSERT(false);
				break;
			}
		} while(word0 != 0xFFFFFFFF);

		debug_i("Sector #0 erased after %u writes", writeCount);

		// Re-mount file system and confirm test file is still present
		spiffs_unmount();
		spiffs_mount();
		auto content = fileGetContent(testFile);
		REQUIRE(testContent == content);
	}
};

void REGISTER_TEST(Spiffs)
{
	registerGroup<SpiffsTest>();
}
