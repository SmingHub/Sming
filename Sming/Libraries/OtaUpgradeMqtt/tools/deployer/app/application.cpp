#include <SmingCore.h>
#include <Network/MqttClient.h>

#ifdef ARCH_HOST
#ifdef __WIN32__
#include <io.h>
#endif

#include <hostlib/CommandLine.h>
#include <sys/stat.h>
#include <Data/Stream/HostFileStream.h>
#endif

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

namespace
{
MqttClient mqtt;

#ifdef ARCH_HOST

size_t charsWriter(const char* buffer, size_t length)
{
	return fwrite(buffer, sizeof(char), length, stdout);
}

template <typename T> void print(const T& arg)
{
	String s(arg);
	m_nputs(s.c_str(), s.length());
}

void println()
{
	m_puts("\r\n");
}

template <typename T> void println(const T& arg)
{
	print(arg);
	println();
}

size_t writePatchVersion(size_t patchVersion, bool useVarInt, ReadWriteStream& output)
{
	size_t written = 0;
	if(useVarInt) {
		while(patchVersion > 0x7f) {
			if(output.write(uint8_t(patchVersion | 0x80)) == 0) {
				return false;
			}
			patchVersion >>= 7;
			written++;
		}
		if(output.write(uint8_t(patchVersion & 0x7f)) == 0) {
			return false;
		}
		written++;
	} else {
		written = output.write(uint8_t(patchVersion));
	}

	return written;
}

static void fileError(IFS::FsBase& fs, const String& filename, const String& operation)
{
	print(F("ERROR: Failed to "));
	print(operation);
	print(F(" file '"));
	print(filename);
	print("': ");
	println(fs.getLastErrorString());
}

bool pack(const String& inputFileName, const String& outputFileName, size_t patchVersion, bool useVarInt)
{
	HostFileStream input;
	if(!input.open(inputFileName)) {
		fileError(input, inputFileName, F("open input"));
		return false;
	}

	HostFileStream output;
	if(!output.open(outputFileName, eFO_CreateNewAlways | eFO_WriteOnly)) {
		fileError(output, outputFileName, F("open output"));
		return false;
	}
	writePatchVersion(patchVersion, useVarInt, output);
	output.copyFrom(&input);
	if(input.getLastError() != FS_OK) {
		fileError(input, inputFileName, F("read from"));
	}
	if(output.getLastError() != FS_OK) {
		fileError(output, outputFileName, F("write to"));
	}

	return true;
}

bool deploy(const String& outputFileName, const String& url)
{
	HostFileStream* output = new HostFileStream();
	if(!output->open(outputFileName)) {
		fileError(*output, outputFileName, F("open output"));
		return false;
	}

	WifiStation.enable(true, false);
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiAccessPoint.enable(false, false);
	WifiEvents.onStationGotIP([url, output](IpAddress ip, IpAddress netmask, IpAddress gateway) {
		Url mqttUrl(url);

		mqtt.connect(mqttUrl, "sming");
		mqtt.setConnectedHandler([mqttUrl, output](MqttClient& client, mqtt_message_t* message) -> int {
			if(message == nullptr) {
				// invalid message received
				return 1;
			}

			if(message->connack.return_code) {
				print(F("ERROR: Connection failed. Reason: "));
				println(mqtt_connect_error_string(mqtt_connect_error_t(message->connack.return_code)));
				System.restart(1000);
				return 0;
			}

			uint8_t retained = 1;
			uint8_t QoS = 2;
			uint8_t flags = uint8_t(retained + (QoS << 1));
			mqtt.publish(mqttUrl.Path.substring(1), output, flags);
			mqtt.setPublishedHandler([](MqttClient& client, mqtt_message_t* message) -> int {
				println(F("Firmware uploaded successfully."));
				System.restart(1000);
				return 0;
			});

			return 0;
		});
	});

	return true;
}

void help()
{
	println();
	println(F("Available commands:"));
	println(F("  pack   fileName.in fileName.out patchVersion <use-varint=1|0>    Creates a package to be deployed on "
			  "firmware upgrade server."));
	println(F("  deploy fileName.out mqttUrl                                      Deploys a deployment package to "
			  "firmware upgrade server."));
	println();
}

/*
 * Return true to continue execution, false to quit.
 */
bool parseCommands()
{
	auto parameters = commandLine.getParameters();
	if(parameters.count() == 0) {
		help();
		return false;
	}

	String cmd = parameters[0].text;

	auto checkParameterCount = [&](unsigned minCount, unsigned maxCount) -> bool {
		if(parameters.count() < 4) {
			print(F("Insufficient"));
		} else if(parameters.count() > 5) {
			print(F("Too many"));
		} else {
			return true;
		}

		print(F(" parameters for '"));
		print(cmd);
		println("'.");
		return false;
	};

	if(cmd == "pack") {
		if(checkParameterCount(4, 5)) {
			auto inputFileName = parameters[1].text;
			auto outputFileName = parameters[2].text;
			auto patchVersion = strtoul(parameters[3].text, nullptr, 0);
			bool useVarInt = false;
			if(parameters.count() > 4) {
				String p(parameters[4].text);
				if(p == "0") {
					useVarInt = false;
				} else if(p == "1") {
					useVarInt = true;
				} else {
					println(F("Invalid setting for useVarInt, must be 1 or 0"));
					return false;
				}
			}
			if(!useVarInt && patchVersion > 0xff) {
				println(F("Patch version number too large for `useVarInt = 0`"));
			} else {
				pack(inputFileName, outputFileName, patchVersion, useVarInt);
			}
			return false; // after packaging the application can be terminated
		}
	} else if(cmd == "deploy") {
		if(checkParameterCount(2, 2)) {
			return deploy(parameters[1].text, parameters[2].text);
		}
	} else {
		print(F("ERROR: Unknown command '"));
		print(cmd);
		println("'.");
	}

	help();
	return false;
}

#endif // ARCH_HOST

} // namespace

void init()
{
	Serial.setTxBufferSize(1024);
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true);

#ifdef ARCH_HOST
	m_setPuts(charsWriter);

	if(!parseCommands()) {
		System.restart(1000);
	}
#endif
}
