/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * application.cpp
 *
 * @author Sept 2019 mikee47 <mike@sillyhouse.net>
 *
 * Sample audio player application
 *
 ****/

#include <SmingCore.h>
#include "AudioPlayer.h"
#include <Network/Http/Websocket/WebsocketResource.h>
#include <ArduinoJson.h>
#include <RtttlJsonListStream.h>
#include <Services/Profiling/CpuUsage.h>
#include <malloc_count.h>

using NotifyCode = AudioPlayer::NotifyCode;

// Usually easiest to define WiFi settings via command line
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID"
#define WIFI_PWD "PleaseEnterPass"
#endif

static HttpServer server;
static SimpleTimer statusTimer;

DEFINE_FSTR(tunes_txt, "tunes.txt");

constexpr unsigned sampleRate = 44100;
static AudioPlayer player;

static Profiling::CpuUsage cpuUsage;

static void showHelp()
{
	Serial.println(_F("\r\n"
					  "RingTone Player\r\n"
					  "\r\n"
					  "Keys to control playback\r\n"
					  "  H     : Show this help\r\n"
					  "  N     : Play next tune\r\n"
					  "  P     : Play previous tune\r\n"
					  "  R     : Rewind to start of current tune\r\n"
					  "  S     : Skip tune\r\n"
					  "  Enter : Play input tune number\r\n"
					  "  Space : Pause/resume playback\r\n"
					  "  +/-   : Adjust speed\r\n"
					  "  L     : List tunes\r\n"
					  "  M     : Select play mode\r\n"
					  "  V     : Select voice"));
}

static void listTunes()
{
	auto curIndex = player.getIndex();
	RingTone::RtttlParser parser;
	if(parser.begin(new FileStream(tunes_txt))) {
		do {
			auto index = parser.getIndex();
			if(index == curIndex) {
				Serial.print('*');
			}
			Serial.print(index);
			Serial.print(": ");
			Serial.println(parser.getTitle());
		} while(parser.nextTune());
	}
}

static void handleCommand(char c)
{
	static int num = -1;
	if(isdigit(c)) {
		if(num < 0) {
			num = c - '0';
		} else {
			num = (num * 10) + c - '0';
		}
		Serial.println();
		Serial.print('>');
		Serial.print(num);
		return;
	}

	switch(tolower(c)) {
	case '\r':
	case '\n':
		if(num >= 0) {
			player.playTune(num);
			num = -1;
		}
		break;

	case 0x1b:
		num = 0;
		Serial.println();
		break;

	case 'n':
		Serial.println(_F(">> Skipping"));
		player.playNextTune();
		break;

	case 'p':
		Serial.println(_F(">> Previous"));
		player.playPreviousTune();
		break;

	case ' ':
		if(player.isStarted()) {
			Serial.println(_F("> Pausing"));
			player.pause();
		} else {
			Serial.print(_F("> Resuming"));
			player.start();
		}
		break;

	case 'r':
		Serial.println(_F("> Rewinding"));
		player.rewind();
		player.start();
		break;

	case 's':
		Serial.println(_F("> Skipping"));
		player.skipTune();
		break;

	case 'l':
		Serial.println();
		listTunes();
		break;

	case 'v': {
		player.nextVoice();
		break;
	}

	case 'm': {
		player.nextMode();
		break;
	}

	case '+':
	case '-': {
		player.adjustSpeed((c == '+') ? 5 : -5);
		break;
	}

	case 'h':
		showHelp();
		break;

	default:
		Serial.print("? ");
		Serial.println(unsigned(c), HEX);
	}
}

static void setJson(JsonObject& json, NotifyCode code, const String& msg = nullptr)
{
	auto obj = json.createNestedObject(AudioPlayer::getNotifyCodeName(code));
	switch(code) {
	case NotifyCode::TuneChanged:
		obj["text"] = player.getCaption();
		obj["index"] = player.getIndex();
		break;
	default:
		if(msg) {
			obj["text"] = msg;
		}
	}
}

static void playerNotify(NotifyCode code, const String& msg)
{
	Serial.print("> ");
	switch(code) {
	case NotifyCode::Info:
		Serial.println(msg);
		break;
	case NotifyCode::Warning:
		Serial.print(_F("Warning: "));
		Serial.println(msg);
		break;
	case NotifyCode::Error:
		Serial.print(_F("Error: "));
		Serial.println(msg);
		break;
	case NotifyCode::TuneChanged:
		Serial.print(_F("Tune: "));
		Serial.println(player.getCaption());
		break;
	case NotifyCode::Started:
		Serial.println(_F("Started"));
		break;
	case NotifyCode::Paused:
		Serial.println(_F("Paused"));
		break;
	case NotifyCode::Stopped:
		Serial.println(_F("Stopped"));
		break;
	case NotifyCode::TuneCompleted:
		Serial.print(_F("Tune complete, duration: "));
		Serial.println(player.getPlayTime().value());
		break;
	case NotifyCode::VoiceChanged:
		Serial.print(_F("Voice: "));
		Serial.println(msg);
		break;
	case NotifyCode::ModeChanged:
		Serial.print(_F("Play mode: "));
		Serial.println(msg);
		break;
	case NotifyCode::SpeedChanged:
		Serial.print(_F("Speed: "));
		Serial.println(msg.toInt() / 100.0);
		break;
	}

	StaticJsonDocument<512> doc;
	auto json = doc.to<JsonObject>();
	setJson(json, code, msg);
	WebsocketConnection::broadcast(Json::serialize(doc));
}

static void broadcastStatus()
{
	//	debug_i("loopIterations = %u, totalCycles = %u, utilisation = %u", cpuUsage.getLoopIterations(),
	//			cpuUsage.getTotalCycles(), cpuUsage.getUtilisation());

	if(WebsocketConnection::getActiveWebsockets().count() > 0) {
		StaticJsonDocument<512> doc;
		auto json = doc.to<JsonObject>();

		json["Time"] = unsigned(player.getPlayTime());

		{
			auto obj = json.createNestedObject("Mem");
			obj["used"] = MallocCount::getCurrent();
			obj["peak"] = MallocCount::getPeak();
			obj["free"] = system_get_free_heap_size();
		}

		auto fillValues = [&](const String& name, const Profiling::MinMax32& values) {
			auto obj = json.createNestedObject(name);
			obj["min"] = values.getMin();
			obj["max"] = values.getMax();
			obj["total"] = values.getTotal();
			obj["count"] = values.getCount();
		};

		fillValues("FillTime", player.fillTimes);

		{
			auto obj = json.createNestedObject("Cpu");
			obj["usage"] = cpuUsage.getUtilisation();
			obj["iterations"] = cpuUsage.getLoopIterations();
			obj["cycles"] = cpuUsage.getElapsedCycles();
			obj["frequency"] = System.getCpuFrequency() * 1000000U;
		}

		json["Clients"] = WebsocketConnection::getActiveWebsockets().count();

		WebsocketConnection::broadcast(Json::serialize(json));
	}

	cpuUsage.reset();
	player.fillTimes.clear();
	MallocCount::resetPeak();
}

static void onReady()
{
	showHelp();

	statusTimer.initializeMs<1000>(broadcastStatus).start();

	player.onNotify(playerNotify);
	if(!player.begin(new FileStream(tunes_txt), sampleRate)) {
		return;
	}

	player.start();
}

static void onSerialData(Stream& source, char arrivedChar, uint16_t availableCharsCount)
{
	handleCommand(arrivedChar);
	Serial.clear(SERIAL_RX_ONLY);
}

static void onFile(HttpRequest& request, HttpResponse& response)
{
	String file = request.uri.getRelativePath();

	debug_i("onFile(%s)", file.c_str());

	if(file[0] == '.') {
		response.code = HTTP_STATUS_FORBIDDEN;
	} else {
		if(file.length() == 0) {
			file = _F("index.html");
		}
		//		response.setCache(86400, true); // It's important to use cache for better performance.
		response.sendFile(file);
	}
}

static void wsConnected(WebsocketConnection& socket)
{
	debug_i("wsConnected %s", socket.getConnection()->getRemoteIp().toString().c_str());

	StaticJsonDocument<512> doc;
	auto json = doc.to<JsonObject>();

	setJson(json, NotifyCode::TuneChanged);
	setJson(json, NotifyCode::VoiceChanged, player.getVoiceName());
	setJson(json, NotifyCode::ModeChanged, player.getModeName());
	setJson(json, NotifyCode::SpeedChanged, String(player.getSpeed()));
	if(player.isStarted()) {
		setJson(json, NotifyCode::Started);
	}
	socket.send(Json::serialize(json));
}

static void wsDisconnected(WebsocketConnection& socket)
{
	debug_i("wsDisconnected %s", socket.getConnection()->getRemoteIp().toString().c_str());
}

static void wsBinaryReceived(WebsocketConnection& socket, uint8_t* data, size_t size)
{
	debug_i("Websocket binary data received, size: %u bytes", size);
}

static void wsMessageReceived(WebsocketConnection& socket, const String& message)
{
	debug_i("WebSocket message received: %s", message.c_str());
	StaticJsonDocument<512> doc;
	if(!Json::deserialize(doc, message)) {
		debug_e("Invalid message: %s", message.c_str());
		return;
	}

	auto json = doc.as<JsonObject>();

	String cmd = json[_F("cmd")].as<const char*>();
	String arg = json[_F("arg")].as<const char*>();

	debug_i("onCommand: '%s', '%s'", cmd.c_str(), arg.c_str());

	if(cmd == _F("prev")) {
		player.playPreviousTune();
	} else if(cmd == _F("next")) {
		player.playNextTune();
	} else if(cmd == _F("skip")) {
		player.skipTune();
	} else if(cmd == _F("stop")) {
		player.stop();
	} else if(cmd == _F("pause")) {
		player.pause();
	} else if(cmd == _F("start")) {
		player.start();
	} else if(cmd == _F("rewind")) {
		player.rewind();
		player.start();
	} else if(cmd == _F("voice")) {
		player.nextVoice();
	} else if(cmd == _F("mode")) {
		player.nextMode();
	} else if(cmd == _F("goto")) {
		if(arg.length() > 0) {
			player.playTune(arg.toInt());
		}
	} else if(cmd == _F("speed")) {
		if(arg.length() > 0) {
			player.setSpeed(arg.toFloat() * 100);
		}
	} else if(cmd == _F("reboot")) {
		playerNotify(AudioPlayer::NotifyCode::Warning, _F("Rebooting..."));
		player.stop();
		i2s_driver_uninstall();
		System.restart(5000);
	}
}

static void onAjaxList(HttpRequest& request, HttpResponse& response)
{
	debug_i("onAjaxList()");

	auto parser = new RingTone::RtttlParser;
	parser->begin(new FileStream(tunes_txt));
	auto stream = new RtttlJsonListStream(_F("tunes"), parser);
	response.sendNamedStream(stream);
}

static void gotIP(IpAddress ip, IpAddress netmask, IpAddress gateway)
{
	server.listen(80);
	server.paths.set(F("/ajax/list"), onAjaxList);
	server.paths.setDefault(onFile);

	// Web Sockets configuration
	auto res = new WebsocketResource();
	res->setConnectionHandler(wsConnected);
	res->setMessageHandler(wsMessageReceived);
	res->setBinaryHandler(wsBinaryReceived);
	res->setDisconnectionHandler(wsDisconnected);
	server.paths.set("/ws", res);

	debug_i("=== WEB SERVER STARTED ===");
	debug_i("%s", ip.toString().c_str());
	debug_i("==============================");
}

void init()
{
	Serial.setTxBufferSize(1024);
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true);
	Serial.onDataReceived(onSerialData);

	// I2S uses standard serial pins, so use alternate pins
	Serial.swap();

	// Network
	WifiStation.enable(true, false);
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiAccessPoint.enable(false, false);
	WifiEvents.onStationGotIP(gotIP);

	spiffs_mount();

	cpuUsage.begin(onReady);
}
