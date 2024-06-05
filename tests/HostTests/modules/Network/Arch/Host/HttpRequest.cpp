#include <HostTests.h>

#include "Network/HttpServer.h"
#include "Network/HttpClient.h"
#include <Platform/Station.h>
#include <IFS/Helpers.h>

namespace
{
struct TestFile {
	const char* name;	 // Name of file requested via HTTP
	const char* realName; // Actual filename on disk (optional, defaults to name)
	MimeType mimeType;
	const char* contentEncoding;

	size_t getSize() const
	{
		return fileGetSize(realName ?: name);
	}
};

TestFile testFiles[]{
	{"bootstrap.min.css", "bootstrap.min.css.gz", MimeType::CSS, "gzip"},
	{"bootstrap.min.css.gz", nullptr, MimeType::UNKNOWN, ""},
	{"index.html", nullptr, MimeType::HTML, ""},
	{"index.js", nullptr, MimeType::JS, ""},
};

} // namespace

class HttpRequestTest : public TestGroup
{
public:
	HttpRequestTest() : TestGroup(_F("HTTP")), server(new HttpServer)
	{
	}

	void execute() override
	{
		if(!WifiStation.isConnected()) {
			Serial.println("No network, skipping tests");
			return;
		}

		REQUIRE(fwfs_mount(Storage::findPartition("fwfs_httprequest")));

		server->listen(80);
		server->paths.setDefault([](HttpRequest& request, HttpResponse& response) {
			auto path = request.uri.getRelativePath();
			bool ok = response.sendFile(path);
			(void)ok;
			debug_i("Request from '%s' for '%s': %s", request.uri.Host.c_str(), path.c_str(), ok ? "OK" : "FAIL");
		});

		requestNextFile();
		pending();
	}

	void requestNextFile()
	{
		if(fileIndex >= ARRAY_SIZE(testFiles)) {
			shutdown();
			return;
		}

		auto& file = testFiles[fileIndex++];
		Url url;
		url.Host = WifiStation.getIP().toString();
		url.Port = 80;
		url.Path = String('/') + file.name;

		auto req = new HttpRequest(url);
		req->onRequestComplete([this, file](HttpConnection& connection, bool success) -> int {
			auto response = connection.getResponse();
			debug_i("Client received '%s'", connection.getRequest()->uri.toString().c_str());
			Serial.print(response->toString());

			REQUIRE(response->code == HTTP_STATUS_OK);
			REQUIRE(response->headers[HTTP_HEADER_CONTENT_TYPE] == toString(file.mimeType));
			REQUIRE(response->headers[HTTP_HEADER_CONTENT_ENCODING] == file.contentEncoding);
			REQUIRE(response->headers[HTTP_HEADER_CONTENT_LENGTH] == String(file.getSize()));

			Serial.println();

			requestNextFile();
			return 0;
		});
		bool ok = client.send(req);
		debug_i("Requested '%s': %s", file.name, ok ? "OK" : "FAIL");
	}

	void shutdown()
	{
		server->shutdown();
		server = nullptr;
		timer.initializeMs<1000>([this]() { complete(); });
		timer.startOnce();
	}

private:
	HttpServer* server{nullptr};
	unsigned fileIndex{0};
	HttpClient client;
	Timer timer;
};

void REGISTER_TEST(HttpRequest)
{
	registerGroup<HttpRequestTest>();
}
