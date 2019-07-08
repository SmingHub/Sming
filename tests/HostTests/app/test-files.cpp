#include "common.h"

IMPORT_FSTR(testContent, SMING_HOME "/../Readme.md");

void test_files()
{
	testGroup("files");

	debug_i("testContent.length = 0x%08x", testContent.length());

	DEFINE_FSTR_LOCAL(testFileName, "test.txt");
	spiffs_mount();

	int res, pos, size;

	//
	startTest("Initial position and size");
	res = fileSetContent(testFileName, testContent);
	debug_i("fileSetContent() returned %d", res);
	assert(size_t(res) == testContent.length());
	auto file = fileOpen(testFileName, eFO_ReadWrite);
	size = fileSeek(file, 0, eSO_FileEnd);
	pos = fileSeek(file, 100, eSO_FileStart);
	debug_i("pos = %d, size = %d", pos, size);
	assert(pos == 100 && size_t(size) == testContent.length());

	startTest("Reduce file sizes");
	res = fileTruncate(file, 555);
	pos = fileTell(file);
	size = fileSeek(file, 0, eSO_FileEnd);
	debug_i("res = %d, pos = %d, size = %d", res, pos, size);
	assert(res == 0 && pos == 100 && size == 555);

	startTest("Increase file size");
	res = fileTruncate(file, 12345);
	size = fileSeek(file, 0, eSO_FileEnd);
	debug_i("res = %d, size = %d", res, size);
	assert(res < 0 && size == 555);

	startTest("Close file");
	fileClose(file);
	size = fileGetSize(testFileName);
	debug_i("size = %u", size);
	assert(size == 555);

	startTest("Truncate by file name, increase size");
	fileSetContent(testFileName, testContent);
	res = fileTruncate(testFileName, 34500);
	size = fileGetSize(testFileName);
	debug_i("fileTruncate() returned %d, size = %d", res, size);
	assert(res < 0 && size_t(size) == testContent.length());

	startTest("Truncate by file name, reduce size");
	res = fileTruncate(testFileName, 345);
	size = fileGetSize(testFileName);
	debug_i("fileTruncate() returned %d, size = %d", res, size);
	assert(res == 0 && size == 345);

	startTest("Truncate read-only file stream");
	fileSetContent(testFileName, testContent);
	FileStream fs(testFileName);
	res = fs.truncate(100);
	pos = fs.getPos();
	size = fs.getSize();
	debug_i("fs.truncate() returned %d, pos = %d, size = %d", res, pos, size);
	assert(res == int(false) && pos == 0 && size_t(size) == testContent.length());
	fs.close();
	size = fileGetSize(testFileName);
	debug_i("Actual file size = %d", size);
	assert(size_t(size) == testContent.length());

	startTest("Truncate read/write file stream");
	fs.open(testFileName, eFO_ReadWrite);
	fs.seek(50);
	res = fs.truncate(100);
	pos = fs.getPos();
	size = fs.getSize();
	debug_i("fs.truncate() returned %d, pos = %d, size = %d", res, pos, size);
	assert(res == int(true) && pos == 50 && size == 100);
	fs.close();
	size = fileGetSize(testFileName);
	debug_i("Actual file size = %d", size);
	assert(size == 100);

	startTest("Seek file stream past end of file");
	fs.open(testFileName, eFO_ReadWrite);
	res = fs.seekFrom(101, eSO_FileStart);
	pos = fs.getPos();
	size = fs.getSize();
	debug_i("fs.seekFrom() returned %d, pos = %d, size = %d", res, pos, size);
	assert(res == 100 && pos == 100 && size == 100);
	fs.close();
	size = fileGetSize(testFileName);
	debug_i("Actual file size = %d", size);
	assert(size == 100);
}
