#pragma once

#include <SmingCore.h>
#include <Data/CStringArray.h>
#include <Dial/Client.h>
#include <Network/UPnP/DeviceHost.h>
#include <Network/UPnP/Soap.h>

using namespace rapidxml;

#define VIERA_COMMAND_MAP(XX)                                                                                          \
	/* action, description */                                                                                          \
	XX(CH_DOWN, "channel down")                                                                                        \
	XX(CH_UP, "channel up")                                                                                            \
	XX(VOLUP, "volume up")                                                                                             \
	XX(VOLDOWN, "volume down")                                                                                         \
	XX(POWER, "power off")                                                                                             \
	XX(MUTE, "mute")                                                                                                   \
	XX(TV, "TV")                                                                                                       \
	XX(CHG_INPUT, "AV")                                                                                                \
	XX(HOME, "home screen")                                                                                            \
	XX(APPS, "apps")                                                                                                   \
	XX(GUIDE, "guide")                                                                                                 \
	XX(RED, "red button")                                                                                              \
	XX(GREEN, "green")                                                                                                 \
	XX(YELLOW, "yellow")                                                                                               \
	XX(BLUE, "blue")                                                                                                   \
	XX(VTOOLS, "VIERA tools")                                                                                          \
	XX(CANCEL, "Cancel / Exit")                                                                                        \
	XX(SUBMENU, "Option")                                                                                              \
	XX(RETURN, "Return")                                                                                               \
	XX(ENTER, "Control Center click / enter")                                                                          \
	XX(RIGHT, "Control RIGHT")                                                                                         \
	XX(LEFT, "Control LEFT")                                                                                           \
	XX(UP, "Control UP")                                                                                               \
	XX(DOWN, "Control DOWN")                                                                                           \
	XX(3D, "3D button")                                                                                                \
	XX(SD_CARD, "SD-card")                                                                                             \
	XX(DISP_MODE, "Display mode / Aspect ratio")                                                                       \
	XX(MENU, "Menu")                                                                                                   \
	XX(INTERNET, "VIERA connect")                                                                                      \
	XX(VIERA_LINK, "VIERA link")                                                                                       \
	XX(EPG, "Guide / EPG")                                                                                             \
	XX(TEXT, "Text / TTV")                                                                                             \
	XX(STTL, "STTL / Subtitles")                                                                                       \
	XX(INFO, "info")                                                                                                   \
	XX(INDEX, "TTV index")                                                                                             \
	XX(HOLD, "TTV hold / image freeze")                                                                                \
	XX(R_TUNE, "Last view")                                                                                            \
	XX(REW, "rewind")                                                                                                  \
	XX(PLAY, "play")                                                                                                   \
	XX(FF, "fast forward")                                                                                             \
	XX(SKIP_PREV, "skip previous")                                                                                     \
	XX(PAUSE, "pause")                                                                                                 \
	XX(SKIP_NEXT, "skip next")                                                                                         \
	XX(STOP, "stop")                                                                                                   \
	XX(REC, "record")

/* A quick way to check if an app is existing on your TV, if it does not support X_GetAppList is to check
 * whether the application icon is present.
 *
 * For example: http://192.168.2.222:55000/nrc/app_icon/0010000100000001 will get the icon for Amazon Prime Video
 */

#define VIERA_APP_MAP(XX)                                                                                              \
	/** id, name, code */                                                                                              \
	XX(NETFLIX, "Netflix", 0010000200000001)                                                                           \
	XX(YOUTUBE, "YouTube", 0070000200000001)                                                                           \
	XX(MEDIA_PLAYER, "Media Player", 0387878700000032)                                                                 \
	XX(AMAZON, "Amazon Prime Video", 0010000100000001)                                                                 \
	XX(PLEX, "Plex", 0076010507000001)                                                                                 \
	XX(BBC_IPLAYER, "BBC iPlayer", 0020000A00170010)                                                                   \
	XX(BBC_NEWS, "BBC News", 0020000A00170006)                                                                         \
	XX(BBC_SPORT, "BBC Sport", 0020000A00170007)                                                                       \
	XX(ITV_HUB, "ITV Hub", 0387878700000124)                                                                           \
	XX(TUNE_INE, "TuneIn", 0010001800000001)                                                                           \
	XX(ACCU_WATHER, "AccuWeather", 0070000C00000001)                                                                   \
	XX(ALL_4, "All 4", 0387878700000125)                                                                               \
	XX(DEMAND_5, "Demand 5", 0020009300000002)                                                                         \
	XX(RAKUTEN, "Rakuten TV", 0020002A00000001)                                                                        \
	XX(CHILI, "CHILI", 0020004700000001)                                                                               \
	XX(STV_PLAYER, "STV Player", 0387878700000132)                                                                     \
	XX(DIGITAL_CONCERT_HALL, "Digital Concert Hall", 0076002307170001)                                                 \
	XX(APPS_MARKET, "Apps Market", 0387878700000102)                                                                   \
	XX(BROWSER, "Browser", 0077777700140002)                                                                           \
	XX(CALENDAR, "Calendar", 0387878700150020)                                                                         \
	XX(VIERA_LINK, "VIERA Link", 0387878700000016)                                                                     \
	XX(RECORDED_TV, "Recorded TV", 0387878700000013)                                                                   \
	XX(FREEVIEW_CATCH_UP, "Freeview Catch Up", 0387878700000109)

namespace Panasonic
{
namespace VieraTV
{
enum class Action {
#define XX(name, description) ACTION_##name,
	VIERA_COMMAND_MAP(XX)
#undef XX
};

enum class ApplicationId {
#define XX(id, name, code) APP_##id,
	VIERA_APP_MAP(XX)
#undef XX
};

String toString(enum Action a);
String toString(enum ApplicationId a);

class Client : public Dial::Client
{
public:
	using GetMuteCallback = Delegate<void(bool muted)>;
	using GetVolumeCallback = Delegate<void(int volume)>;

	struct Command {
		enum class Type {
			REMOTE,
			RENDER,
		};

		Type type;
		String name; // How device identifies itself
		XML::Document* params;
	};

	Client(size_t maxDescriptionSize = 4096) : Dial::Client(maxDescriptionSize)
	{
	}

	using Dial::Client::connect;

	bool connect(ConnectedCallback callback,
				 const String& type = "urn:panasonic-com:device:p00RemoteController:1") override
	{
		return Dial::Client::connect(callback, type);
	}

	/**
	 * Send a command to the TV
	 *
	 * @param action command Command from codes.txt
	 */
	bool sendCommand(Action action);

	/**
	 * Send a change HDMI input to the TV
	 *
	 * @param input
	 */
	bool switchToHdmi(size_t input);

	/**
	 * Send command to open app on the TV
	 *
	 * @param id
	 */
	bool launch(enum ApplicationId id)
	{
		return launch(toString(id));
	}

	/**
	 * Send command to open app on the TV
	 *
	 * @param {String} applicationId appId from codes.txt
	 */
	bool launch(const String& applicationId);

	/**
	 * Get volume from TV
	 *
	 * @param callback
	 * @return bool - true on success false otherwise
	 */
	bool getVolume(GetVolumeCallback onVolume);

	/**
	 * Set volume
	 *
	 * @param {Int} volume Desired volume in range from 0 to 100
	 */
	bool setVolume(size_t volume);

	/**
	 * Get the current mute setting
	 *
	 * @param callback
	 * @return bool - true on success false otherwise
	 */
	bool getMute(GetMuteCallback onMute);

	/**
	 * Set mute to on/off
	 *
	 * @param {Boolean} enable The value to set mute to
	 */
	bool setMute(bool enable);

private:
	SOAP::Envelope envelope;
	XML::Node* actionTag = nullptr;
	XML::Document paramsDoc;

	bool sendRequest(Command command, RequestCompletedDelegate requestCallack = nullptr);

	bool setParams(Command& cmd, const String& text)
	{
		cmd.params = &paramsDoc;
		cmd.params->parse<0>((char*)text.c_str());
		return true;
	}
};

} // namespace VieraTV

} // namespace Panasonic
