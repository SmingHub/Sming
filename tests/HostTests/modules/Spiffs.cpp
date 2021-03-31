#include <HostTests.h>
#include <Storage.h>

#ifdef ARCH_HOST
#include <Storage/FileDevice.h>
#include <IFS/SPIFFS/FileSystem.h>
using FileSystem = IFS::FileSystem;
#endif

class SpiffsTest : public TestGroup
{
public:
	SpiffsTest() : TestGroup(_F("SPIFFS"))
	{
	}

	void execute() override
	{
#ifdef ARCH_HOST
		TEST_CASE("Check Spiffsgen.py")
		{
			checkSpiffsGen();
		}
#endif

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
		fileFreeFileSystem();
		if(!spiffs_mount()) {
			debug_e("SPIFFS mount failed");
			return;
		}

		DEFINE_FSTR_LOCAL(testFile, "testfile");
		DEFINE_FSTR_LOCAL(testContent, "Some test content to write to a file");

		auto part = *Storage::findPartition(Storage::Partition::SubType::Data::spiffs);
		REQUIRE(part.name() == "spiffs0");

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
		fileFreeFileSystem();
		spiffs_mount();
		auto content = fileGetContent(testFile);
		REQUIRE(testContent == content);
	}

#ifdef ARCH_HOST
	/*
	 * Verify that a legacy volume (i.e. one generated with spiffy before IFS was introduced)
	 * can still be read correctly.
	 * 
	 * Compare content of all files to ensure they are identical.
	 */
	void checkSpiffsGen()
	{
		FileSystem::Info info;
		int err = fileGetSystemInfo(info);
		CHECK(err >= 0);
		debug_i("fs attr = %s", toString(info.attr).c_str());

		FileSystem* fsOld;
		if(info.attr[FileSystem::Attribute::NoMeta]) {
			fsOld = mountSpiffsFromFile("old", "spiffsgen/spiff_rom_orig.bin");
		} else {
			fsOld = mountSpiffsFromFile("old", "spiffsgen/spiff_rom_meta.bin");
		}
		auto fsNew = mountSpiffsFromFile("new", "out/spiff_rom_test.bin");

		readCheck(fsOld, fsNew);

		delete fsNew;
		delete fsOld;

		delete Storage::findDevice("new");
		delete Storage::findDevice("old");
	}

	FileSystem* mountSpiffsFromFile(const String& tag, const String& filename)
	{
		auto& hfs = IFS::Host::getFileSystem();
		auto f = hfs.open(filename, IFS::File::ReadOnly);
		if(f < 0) {
			debug_e("Failed to open '%s': %s", filename.c_str(), hfs.getErrorString(f).c_str());
			return nullptr;
		}
		auto dev = new Storage::FileDevice(tag, hfs, f);
		Storage::registerDevice(dev);
		auto part = dev->createPartition(tag, Storage::Partition::SubType::Data::spiffs, 0, dev->getSize(),
										 Storage::Partition::Flag::readOnly);

		auto fs = new IFS::SPIFFS::FileSystem(part);
		int err = fs->mount();
		if(err < 0) {
			debug_e("SPIFFS mount '%s' failed: %s", tag.c_str(), fs->getErrorString(err).c_str());
			delete fs;
			fs = nullptr;
			delete dev;
		}

		debug_i("Mounted '%s' as '%s'", filename.c_str(), tag.c_str());
		return FileSystem::cast(fs);
	}

	void readCheck(IFS::FileSystem* fsOld, IFS::FileSystem* fsNew)
	{
		DirHandle dir{};
		int res = fsOld->opendir(nullptr, dir);
		if(res < 0) {
			debug_e("opendir failed: %s", fsOld->getErrorString(res).c_str());
			TEST_ASSERT(false);
			return;
		}

		FileNameStat stat;
		while((res = fsOld->readdir(dir, stat)) >= 0) {
			FileStat statNew;
			fsNew->stat(stat.name.buffer, &statNew);
			debug_i("File '%s' size %u / %u", stat.name.buffer, stat.size, statNew.size);

			String oldContent = fsOld->getContent(stat.name.buffer);
			CHECK(oldContent);
			String newContent = fsNew->getContent(stat.name.buffer);
			CHECK(newContent);

			CHECK_EQ(newContent, oldContent);
		}

		CHECK_EQ(res, IFS::Error::NoMoreFiles);
		debug_i("readdir(): %s", fsOld->getErrorString(res).c_str());

		fsOld->closedir(dir);
	}
#endif
};

void REGISTER_TEST(Spiffs)
{
	registerGroup<SpiffsTest>();
}
