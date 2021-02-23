#include <HostTests.h>

#include "Network/HttpServer.h"
#include "Network/HttpClient.h"
#include <Platform/Station.h>
#include <IFS/Helpers.h>

class HttpRequestTest : public TestGroup
{
public:
	HttpRequestTest() : TestGroup(_F("HTTP"))
	{
		server = new HttpServer;
	}

	void execute() override
	{
		auto fs = IFS::createFirmwareFilesystem(*Storage::findPartition(Storage::Partition::SubType::Data::fwfs));
		CHECK(fs != nullptr);
		CHECK(fs->mount() == FS_OK);
		fileSetFileSystem(fs);

		server->listen(80);
		server->paths.setDefault(HttpPathDelegate(&HttpRequestTest::onFile, this));

		Url url;
		url.Host = WifiStation.getIP().toString();
		url.Port = 80;
		url.Path = "/bootstrap.min.css";

		auto req = new HttpRequest(url);
		req->onRequestComplete([this](HttpConnection& connection, bool success) -> int {
			auto response = connection.getResponse();
			Serial.print(F("Received "));
			Serial.println(connection.getRequest()->uri.toString());
			Serial.print(response->toString());

			REQUIRE(response->code == HTTP_STATUS_OK);
			REQUIRE(response->headers[HTTP_HEADER_CONTENT_TYPE] == toString(MimeType::CSS));

			Serial.println();

			shutdown();
			return 0;
		});
		client.send(req);

		pending();
	}

	void shutdown()
	{
		server->shutdown();
		server = nullptr;
		timer.initializeMs<1000>([this]() { complete(); });
		timer.startOnce();
	}

	void onFile(HttpRequest& request, HttpResponse& response)
	{
		String file = request.uri.getRelativePath();

		if(file[0] == '.')
			response.code = HTTP_STATUS_FORBIDDEN;
		else {
			response.setCache(86400, true); // It's important to use cache for better performance.
			response.sendFile(file);
		}
	}

private:
	HttpServer* server{nullptr};
	HttpClient client;
	Timer timer;
};

void REGISTER_TEST(HttpRequest)
{
	// Currently only supported for Host CI
#if defined(ARCH_HOST) && defined(__linux__)
	registerGroup<HttpRequestTest>();
#endif
}
