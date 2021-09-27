/*
 * Webserver demo using IFS
 *
 */

#include <SmingCore.h>
#include <Data/Stream/MemoryDataStream.h>
#include <Data/Stream/IFS/DirectoryTemplate.h>
#include <Data/Stream/IFS/HtmlDirectoryTemplate.h>
#include <Data/Stream/IFS/JsonDirectoryTemplate.h>
#include <Storage/ProgMem.h>
#include <LittleFS.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put your SSID and password here
#define WIFI_PWD "PleaseEnterPass"
#endif

namespace
{
#ifdef ENABLE_FLASHSTRING_IMAGE
IMPORT_FSTR(fwfsImage, PROJECT_DIR "/out/fwfs1.bin")
#endif

IMPORT_FSTR(listing_html, PROJECT_DIR "/resource/listing.html")
IMPORT_FSTR(listing_txt, PROJECT_DIR "/resource/listing.txt")
IMPORT_FSTR(listing_json, PROJECT_DIR "/resource/listing.json")

HttpServer server;
FtpServer ftp;
int requestCount;

/*
 * Handle any custom fields here
 */
String getValue(const char* name)
{
	if(FS("webpage") == name) {
		return "https://github.com/SmingHub/Sming";
	}

	if(FS("request-count") == name) {
		return String(requestCount); // Doesn't require escaping
	}

	return nullptr;
}

void onFile(HttpRequest& request, HttpResponse& response)
{
	++requestCount;

	String file = request.uri.getRelativePath();

	FileStat stat;
	if(fileStats(file, stat) < 0) {
		response.code = HTTP_STATUS_INTERNAL_SERVER_ERROR;
		return;
	}

	if(stat.isDir()) {
		auto dir = new Directory;
		IFS::DirectoryTemplate* tmpl;
		String fmt = request.uri.Query["format"];
		if(fmt.equalsIgnoreCase("json")) {
			auto source = new FlashMemoryStream(listing_json);
			tmpl = new IFS::JsonDirectoryTemplate(source, dir);
		} else if(fmt.equalsIgnoreCase("text")) {
			auto source = new FlashMemoryStream(listing_txt);
			tmpl = new IFS::DirectoryTemplate(source, dir);
		} else {
			auto source = new FlashMemoryStream(listing_html);
			tmpl = new IFS::HtmlDirectoryTemplate(source, dir);
		}
		tmpl->onGetValue(getValue);
		dir->open(file);
		tmpl->gotoSection(0);
		response.sendDataStream(tmpl, tmpl->getMimeType());
	} else {
		//	response.setCache(86400, true); // It's important to use cache for better performance.
		auto stream = new FileStream(stat);
		if(!stream->isValid()) {
			stream->open(file);
		}
		if(stat.compression.type == IFS::Compression::Type::GZip) {
			response.headers[HTTP_HEADER_CONTENT_ENCODING] = F("gzip");
		} else if(stat.compression.type != IFS::Compression::Type::None) {
			debug_e("Unsupported compression type: %u", stat.compression.type);
		}

		auto mimeType = ContentType::fromFullFileName(file.c_str(), MIME_TEXT);
		response.sendDataStream(stream, mimeType);
	}
}

void startWebServer()
{
	server.listen(80);
	server.paths.setDefault(onFile);

	Serial.println("\r\n=== WEB SERVER STARTED ===");
	Serial.println(WifiStation.getIP());
	Serial.println("==============================\r\n");
}

void gotIP(IpAddress ip, IpAddress netmask, IpAddress gateway)
{
	startWebServer();
}

bool initFileSystem()
{
	fileFreeFileSystem();

#if DEBUG_BUILD
	auto freeheap = system_get_free_heap_size();
#endif
	debug_i("1: heap = %u", freeheap);

#ifdef ENABLE_FLASHSTRING_IMAGE
	// Create a partition wrapping some flashstring data
	auto part = Storage::progMem.createPartition(F("fwfsMem"), fwfsImage, Storage::Partition::SubType::Data::fwfs);
#else
	auto part = Storage::findDefaultPartition(Storage::Partition::SubType::Data::fwfs);
#endif

	IFS::IFileSystem* fs;
#ifdef FWFS_HYBRID
	// Create a read/write filesystem
	auto spiffsPart = Storage::findDefaultPartition(Storage::Partition::SubType::Data::spiffs);
	fs = IFS::createHybridFilesystem(part, spiffsPart);
#else
	// Read-only
	fs = IFS::createFirmwareFilesystem(part);
#endif
	debug_i("2: heap = -%u", freeheap - system_get_free_heap_size());

	if(fs == nullptr) {
		debug_e("Failed to created filesystem object");
		return false;
	}

	int res = fs->mount();
	debug_i("3: heap = -%u", freeheap - system_get_free_heap_size());

	debug_i("mount() returned %d (%s)", res, fs->getErrorString(res).c_str());

	if(res < 0) {
		delete fs;
		return false;
	}

	fileSetFileSystem(fs);

	debug_i("File system initialised");
	return true;
}

void printDirectory(const char* path)
{
	auto printStream = [](IDataSourceStream& stream) {
		// Use an intermediate memory stream so debug information doesn't get mixed into output
		//		MemoryDataStream mem;
		//		mem.copyFrom(&stream);
		//		Serial.copyFrom(&mem);
		Serial.copyFrom(&stream);
	};

	{
		auto dir = new Directory;
		if(!dir->open(path)) {
			debug_e("Open '%s' failed: %s", path, dir->getLastErrorString().c_str());
			delete dir;
			return;
		}

		auto source = new FlashMemoryStream(listing_txt);
		IFS::DirectoryTemplate tmpl(source, dir);
		printStream(tmpl);
	}

	{
		auto dir = new Directory;
		if(!dir->open(path)) {
			debug_e("Open '%s' failed: %s", path, dir->getLastErrorString().c_str());
			delete dir;
			return;
		}

		auto source = new FlashMemoryStream(listing_json);
		IFS::JsonDirectoryTemplate tmpl(source, dir);
		printStream(tmpl);
	}
}

void copySomeFiles()
{
	auto part = *Storage::findPartition(Storage::Partition::SubType::Data::fwfs);
	if(!part) {
		return;
	}
	auto fs = IFS::createFirmwareFilesystem(part);
	if(fs == nullptr) {
		return;
	}
	fs->mount();

	IFS::Directory dir(fs);
	if(!dir.open()) {
		return;
	}

	while(dir.next()) {
		auto& stat = dir.stat();
		if(stat.isDir()) {
			continue;
		}
		IFS::File src(fs);
		auto filename = stat.name.c_str();
		if(src.open(filename)) {
			File dst;
			if(dst.open(filename, File::CreateNewAlways | File::WriteOnly)) {
				auto len =
					src.readContent([&dst](const char* buffer, size_t size) -> int { return dst.write(buffer, size); });
				(void)len;
				debug_w("Wrote '%s', %d bytes", filename, len);
				if(!dst.settime(stat.mtime)) {
					Serial.print(F("settime() failed: "));
					Serial.println(dst.getLastErrorString());
				}
				if(!dst.setcompression(stat.compression)) {
					Serial.print(F("setcompression() failed: "));
					Serial.println(dst.getLastErrorString());
				}
				if(!dst.setacl(stat.acl)) {
					Serial.print(F("setacl() failed: "));
					Serial.println(dst.getLastErrorString());
				}
			} else {
				debug_w("%s", dst.getLastErrorString().c_str());
			}
		}
	}
}

bool isVolumeEmpty()
{
	Directory dir;
	dir.open();
	return !dir.next();
}

void fstest()
{
	// Various ways to initialise a filesystem

	/*
	 * Mount regular SPIFFS volume
	 */
	// spiffs_mount();

	/*
	 * Mount LittleFS volume
	 */
	// lfs_mount();

	/*
	 * Mount default Firmware Filesystem
	 */
	// fwfs_mount();

	/*
	 * Mount default FWFS/SPIFFS as hybrid
	 */
	// hyfs_mount();

	/*
	 * Explore some alternative methods of mounting filesystems
	 */
	initFileSystem();

	if(isVolumeEmpty()) {
		Serial.print(F("Volume appears to be empty, writing some files...\r\n"));
		copySomeFiles();
	}

	printDirectory(nullptr);
}

} // namespace

void init()
{
#if DEBUG_BUILD
	Serial.begin(COM_SPEED_SERIAL);

	Serial.systemDebugOutput(true);
	debug_i("\n\n********************************************************\n"
			"Hello\n");
#endif

	// Delay at startup so terminal gets time to start
	auto timer = new AutoDeleteTimer;
	timer->initializeMs<1000>(fstest);
	timer->startOnce();

	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiAccessPoint.enable(false);

	WifiEvents.onStationGotIP(gotIP);
}
