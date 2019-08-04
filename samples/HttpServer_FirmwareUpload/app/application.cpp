#include <SmingCore.h>

#include <Data/Stream/LimitedMemoryStream.h>
#include <MultipartParser/HttpMultipartResource.h>
#include <Data/Stream/RbootOutputStream.h>
#include <sodium/crypto_sign.h>
#include "FirmwareVerificationKey.h"

HttpServer server;
String lastModified;

class SignedRbootOutputStream : public RbootOutputStream
{
	const uint8_t* const verificationKey_;

	struct {
		uint32_t magic;
		uint32_t loadAddress;
		uint8_t signature[crypto_sign_BYTES];
	} header_;

	uint8_t* headerPtr_;
	size_t missingHeaderBytes_;
	static const uint32_t HEADER_MAGIC_EXPECTED = 0xf01af02a;

	bool ok_;
	bool closeCalled_ = false;
	uint32_t startAddress_;

	crypto_sign_state verifierState_;
	String errorMessage_;

	void setError(const char* message)
	{
		ok_ = false;
		errorMessage_ = message;
		Serial.println(errorMessage_);
	}

public:
	SignedRbootOutputStream(int32_t startAddress, size_t maxLength, const uint8_t* verificationKey)
		: RbootOutputStream(startAddress, maxLength), verificationKey_(verificationKey),
		  headerPtr_(reinterpret_cast<uint8_t*>(&header_)), missingHeaderBytes_(sizeof(header_)), ok_(true),
		  startAddress_(startAddress)
	{
		crypto_sign_init(&verifierState_);
	}

	bool ok() const
	{
		return ok_;
	}
	const String& errorMessage() const
	{
		return errorMessage_;
	}

	size_t write(const uint8_t* data, size_t size)
	{
		size_t consumed = 0;
		if(ok_) {
			if(missingHeaderBytes_ > 0) {
				const size_t chunkSize = std::min(size, missingHeaderBytes_);
				memcpy(headerPtr_, data, chunkSize);
				headerPtr_ += chunkSize;
				missingHeaderBytes_ -= chunkSize;
				data += chunkSize;
				size -= chunkSize;
				consumed += chunkSize;
				if(missingHeaderBytes_ == 0) {
					Serial.printf("Receive image for load address 0x%08X, slot starts at 0x%08X\n", header_.loadAddress,
								  startAddress_);
					const bool magicOk = (header_.magic == HEADER_MAGIC_EXPECTED);
					const bool loadAddressOk = ((header_.loadAddress & 0x000FFFFF) == (startAddress_ & 0x000FFFFF));
					ok_ = magicOk && loadAddressOk;
					if(!ok_) {
						setError(magicOk ? "Unexpected load address. Try image for other slot."
										 : "Invalid image received.");
						return 0;
					}

					init();
				}
			}

			if(size > 0) {
				crypto_sign_update(&verifierState_, static_cast<const unsigned char*>(data), size);
				consumed += RbootOutputStream::write(data, size);
			}
		}
		return consumed;
	}

	bool close()
	{
		if(closeCalled_) {
			return true;
		}
		closeCalled_ = true;

		Serial.printf("SignedRbootOutputStream::close()\n");
		if(ok_) {
			if(!RbootOutputStream::close()) {
				return false;
			}

			Serial.printf("Verify signature\n");
			const bool signatureMatch =
				(crypto_sign_final_verify(&verifierState_, header_.signature, verificationKey_) == 0);
			if(!signatureMatch) {
				// destroy start sector of updated ROM to avoid accidental booting an unsanctioned firmware
				spi_flash_erase_sector(startAddress / SECTOR_SIZE);
				setError("Signature mismatch");
			}
		}
		return ok_;
	}
};

void onIndex(HttpRequest& request, HttpResponse& response)
{
	TemplateFileStream* tmpl = new TemplateFileStream("index.html");
	auto& vars = tmpl->variables();
	response.sendNamedStream(tmpl);
}

void onFile(HttpRequest& request, HttpResponse& response)
{
	if(lastModified.length() > 0 && request.headers[HTTP_HEADER_IF_MODIFIED_SINCE].equals(lastModified)) {
		response.code = HTTP_STATUS_NOT_MODIFIED;
		return;
	}

	String file = request.uri.getRelativePath();
	Serial.printf("Requested file: %s\n", file.c_str());
	if(file[0] == '.')
		response.code = HTTP_STATUS_FORBIDDEN;
	else {
		if(lastModified.length() > 0) {
			response.headers[HTTP_HEADER_LAST_MODIFIED] = lastModified;
		}

		response.setCache(86400, true); // It's important to use cache for better performance.
		response.sendFile(file);
	}
}

int onUpload(HttpServerConnection& connection, HttpRequest& request, HttpResponse& response)
{
	Serial.printf("onUpload\n");
	ReadWriteStream* file = request.files["firmware"];
	if(file == nullptr) {
		debug_e("Something went wrong with the file upload");
		return 1;
	}

	SignedRbootOutputStream* uploadStream = static_cast<SignedRbootOutputStream*>(file);
	uploadStream->close();

	if(uploadStream->ok()) {
		rboot_config bootConfig = rboot_get_config();
		uint8_t slot = bootConfig.current_rom;
		slot = (slot == 0 ? 1 : 0);
		Serial.printf("Firmware updated, rebooting to rom %d...\r\n", slot);
		rboot_set_current_rom(slot);
		System.restart(5); // defer the restart with 5 seconds to give time to the web server to return the response

		response.sendFile("restart.html");
	} else {
		response.code = HTTP_STATUS_BAD_REQUEST;
		response.setContentType(MIME_HTML);
		String html = "<H2 color='#444'>" + uploadStream->errorMessage() + "</H2>";
		response.headers[HTTP_HEADER_CONTENT_LENGTH] = html.length();
		response.sendString(html);
	}
	return 0;
}

void fileUploadMapper(HttpFiles& files)
{
	/*
	 * On a normal computer the file uploads are usually using
	 * temporary space on the hard disk or in memory to store the incoming data.
	 *
	 * On an embedded device that is a luxury that we can hardly afford.
	 * Therefore we should define a `map` that specifies explicitly
	 * where a specific form field will be stored.
	 *
	 * If a field is not specified then its content will be discarded.
	 */

	// Get the address where the next firmware should be stored.
	rboot_config bootConfig = rboot_get_config();
	uint8_t slot = bootConfig.current_rom;
	slot = (slot == 0 ? 1 : 0);
	int romStartAddress = bootConfig.roms[slot];

	size_t maxLength = 0; // 0  means that there is no max length.
						  // Set this according to your flash memory layout

	files["firmware"] = new SignedRbootOutputStream(romStartAddress, maxLength, firmwareVerificationKey);
}

void startWebServer()
{
	server.listen(80);
	server.paths.set("/", onIndex);

	HttpMultipartResource* uploadResouce = new HttpMultipartResource(fileUploadMapper, onUpload);
	server.paths.set("/upload", uploadResouce);

	server.paths.setDefault(onFile);
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Enable debug output to serial

	spiffs_mount(); // Mount file system, in order to work with files

	if(fileExist(".lastModified")) {
		// The last modification
		lastModified = fileGetContent(".lastModified");
		lastModified.trim();
	}

	WifiStation.enable(true);
	//WifiStation.config(WIFI_SSID, WIFI_PWD);
	//WifiStation.enableDHCP(true);

	// Run WEB server on system ready
	System.onReady(startWebServer);
}
