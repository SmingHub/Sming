#include <SmingTest.h>

#include <Network/Url.h>
#include <Network/Http/HttpRequest.h>
#include <Network/Http/HttpBodyParser.h>
#include <Data/Stream/UrlencodedOutputStream.h>
#include <Data/Stream/MemoryDataStream.h>

class UrlTest : public TestGroup
{
public:
	UrlTest() : TestGroup(_F("Url"))
	{
	}

	void execute() override
	{
		DEFINE_FSTR_LOCAL(FS_URL1, "http://smingtest.local?param1=1&param2=2#whacky%20races");
		DEFINE_FSTR_LOCAL(FS_URL2, "http://smingtest.local/iocontrol.js?cid=fa373784");
		DEFINE_FSTR_LOCAL(FS_URL3, "/iocontrol.js?cid=81e66a3a");

		TEST_CASE("formUrlParser test")
		{
			auto testUrl = [this](const FlashString& urlText, const char* param) {
				debugf("URL '%s'", String(urlText).c_str());
				Url url(urlText);
				String query = url.Query;
				const char* p = query.c_str();
				++p; // Skip the '?'
				HttpRequest request;
				formUrlParser(request, nullptr, PARSE_DATASTART);
				while(*p != '\0') {
					formUrlParser(request, p, 1);
					++p;
				}
				formUrlParser(request, nullptr, PARSE_DATAEND);
				printParams(request.postParams);
				String cid = request.getPostParameter("cid");
				debugf("cid = %s", cid.c_str());
				REQUIRE(cid == param);
			};

			testUrl(FS_URL1, "");
			testUrl(FS_URL2, "fa373784");
			testUrl(FS_URL3, "81e66a3a");
		}

		HttpRequest request;

		TEST_CASE("HttpRequest getQueryParameter()")
		{
			request.uri = FS_URL2;
			debugf("URL = \"%s\"", request.uri.toString().c_str());
			debugf("cid = %s", request.getQueryParameter("cid").c_str());
		}

		TEST_CASE("HttpRequest postParams test");
		{
			DEFINE_FSTR_LOCAL(FS_serializedParams,
							  "param+1=Mary+had+a+little+lamb%2c&param+2=It%27s+fleece+was+very+red.&param+3=The+"
							  "reason+for+this+was%2c+you+see&param+4=It+had+a+pickaxe+through+its+head.");
			HttpParams params;
			params["param 1"] = "Mary had a little lamb,";
			params["param 2"] = "It's fleece was very red.";
			params["param 3"] = "The reason for this was, you see";
			params["param 4"] = "It had a pickaxe through its head.";
			UrlencodedOutputStream stream(params);
			char buffer[256];
			unsigned n = 0;
			while(!stream.isFinished()) {
				unsigned count = stream.readMemoryBlock(&buffer[n], 11);
				stream.seek(count);
				n += count;
			}
			buffer[n] = '\0';
			REQUIRE(FS_serializedParams == buffer);

			debug_d("Putting it all together...");
			DEFINE_FSTR_LOCAL(FS_serialized,
							  "http://smingtest.local:80/"
							  "iocontrol.js?cid=fa373784&param+1=Mary+had+a+little+lamb%2c&param+2=It%27s+"
							  "fleece+was+very+red.&param+3=The+reason+for+this+was%2c+you+see&param+4=It+"
							  "had+a+pickaxe+through+its+head.#I+told+you+this+would+happen");
			auto& uri = request.uri;
			uri.Query.setMultiple(params);
			uri.Fragment = "I told you this would happen";
			REQUIRE(uri.toString() == FS_serialized);
		}

		TEST_CASE("Alternative construction")
		{
#define MQTT_SERVER "192.168.2.138"
#define MQTT_PORT 1883
#define LOG ""
#define PASS ""
			DEFINE_FSTR_LOCAL(FS_url, "mqtt://" MQTT_SERVER ":1883/");
			Url url(URI_SCHEME_MQTT, F(LOG), F(PASS), F(MQTT_SERVER), MQTT_PORT);
			REQUIRE(url.toString() == FS_url);
		}

		TEST_CASE("Print")
		{
			DEFINE_FSTR_LOCAL(FS_url, "http://simple.anakod.ru/templates/jquery.js.gz");
			DEFINE_FSTR_LOCAL(FS_url_out, "http://simple.anakod.ru:80/templates/jquery.js.gz");
			Url url(FS_url);
			MemoryDataStream stream;
			stream.print(url);
			stream.seekFrom(0, SEEK_SET);
			String s(stream.getStreamPointer(), stream.available());
			REQUIRE(s == FS_url_out);
		}

		TEST_CASE("toString")
		{
			DEFINE_FSTR_LOCAL(FS_url, "http://api.thingspeak.com/update?key=7XXUJWCWYTMXKN3L&field1=347");
			Url uri;
			uri.Host = "api.thingspeak.com";
			uri.Path = "/update";
			uri.Query["key"] = "7XXUJWCWYTMXKN3L";
			int sensorValue = 347;
			uri.Query["field1"] = String(sensorValue);
			REQUIRE(uri.toString() == FS_url);
		}
	}

	void printParams(const HttpParams& params)
	{
		for(unsigned j = 0; j < params.count(); ++j) {
			debug_d("  Param #%u: %s = \"%s\"", j, params.keyAt(j).c_str(), params.valueAt(j).c_str());
		}
	}
};

void REGISTER_TEST(url)
{
	registerGroup<UrlTest>();
}
