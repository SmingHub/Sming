#include <SmingTest.h>

#include "Network/Http/HttpCommon.h"
#include "Network/Http/HttpHeaders.h"
#include "Network/WebConstants.h"
#include <Platform/Timers.h>

class HttpTest : public TestGroup
{
public:
	HttpTest() : TestGroup(_F("HTTP"))
	{
	}

	void execute() override
	{
		testHttpCommon();
		testHttpHeaders();
		profileHttpHeaders();
	}

	void testHttpCommon()
	{
#if DEBUG_VERBOSE_LEVEL == DBG
		for(int i = 0; i < 100; ++i) {
			auto err = http_errno(i);
			debug_d("httpError(%d) = \"%s\", \"%s\"", i, httpGetErrorName(err).c_str(),
					httpGetErrorDescription(err).c_str());
		}

		for(int i = 100; i < 550; ++i) {
			debug_d("http_status(%d) = \"%s\"", i, httpGetStatusText(i).c_str());
		}
#endif

		TEST_CASE("http lookups")
		{
			auto s = httpGetErrorName(HPE_UNKNOWN);
			REQUIRE(s == F("HPE_UNKNOWN"));
			s = httpGetErrorDescription(HPE_INVALID_URL);
			REQUIRE(s == F("invalid URL"));
			s = httpGetStatusText(HTTP_STATUS_TOO_MANY_REQUESTS);
			REQUIRE(s.equalsIgnoreCase(F("too many requests")));
		}
	}

	static void printHeaders(const HttpHeaders& headers)
	{
#if DEBUG_VERBOSE_LEVEL == DBG
		debugf("  count: %d", headers.count());
		for(unsigned i = 0; i < headers.count(); ++i) {
			String s = headers[i];
			m_printHex("  ", s.c_str(), s.length(), 0, 32);
		}
#endif
	}

	void profileHttpHeaders()
	{
		debugf("\nPROFILING");

		// Allocate everything on the heap so we can track memory usage
		auto freeHeap = system_get_free_heap_size();

		auto headersPtr = new HttpHeaders;
		HttpHeaders& headers = *headersPtr;

		// Set header values
		ElapseTimer timer;
		headers[HTTP_HEADER_CONTENT_ENCODING] = _F("gzip");
		headers[HTTP_HEADER_CONTENT_LENGTH] = 6042;
		headers[HTTP_HEADER_ETAG] = _F("00f-3d-179a0-0");
		headers[HTTP_HEADER_CONNECTION] = _F("keep-alive");
		headers[HTTP_HEADER_SERVER] = _F("HttpServer/Sming");
		headers[HTTP_HEADER_CONTENT_TYPE] = ContentType::toString(MIME_JS);
		headers[HTTP_HEADER_CACHE_CONTROL] = F("max-age=31536000, public");
		headers[HTTP_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN] = "*";
		auto standardElapsed = timer.elapsedTime();
		headers[F("X-Served-By")] = _F("Donkey Kong");
		headers[F("Vary")] = _F("Accept-Encoding");
		headers[F("X-Fastly-Request-ID")] = _F("38ef411e0ec3bf681d29d8b4b51f3516d3ef9e03");
		auto totalElapsed = timer.elapsedTime();
		debugf("Set header values");
		debugf("  Elapsed standard: %s, total: %s, heap used: %u", standardElapsed.toString().c_str(),
			   totalElapsed.toString().c_str(), freeHeap - system_get_free_heap_size());

		// Query header value by field name
		auto queryByEnum = [&](HttpHeaderFieldName name) {
			debugf("  header[\"%s\"]: %s", headers.toString(name).c_str(), headers[name].c_str());
		};
		auto queryByString = [&](const String& name) {
			debugf("  header[\"%s\"]: %s", name.c_str(), headers[name].c_str());
		};
		debugf("Query header values");
		timer.start();
		queryByEnum(HTTP_HEADER_CONTENT_ENCODING);
		queryByEnum(HTTP_HEADER_CONTENT_LENGTH);
		queryByEnum(HTTP_HEADER_ETAG);
		queryByEnum(HTTP_HEADER_CONNECTION);
		queryByEnum(HTTP_HEADER_SERVER);
		queryByEnum(HTTP_HEADER_CONTENT_TYPE);
		queryByEnum(HTTP_HEADER_CACHE_CONTROL);
		queryByEnum(HTTP_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN);
		standardElapsed = timer.elapsedTime();
		queryByString("X-Served-By");
		queryByString("Vary");
		queryByString("X-Fastly-Request-ID");
		totalElapsed = timer.elapsedTime();
		debugf("  Elapsed standard: %u, total: %u", standardElapsed, totalElapsed);
		debugf("  Elapsed standard: %s, total: %s", standardElapsed.toString().c_str(),
			   totalElapsed.toString().c_str());

		// Print header values - accessed by index
		debugf("Printing %u headers", headers.count());
		timer.start();
		printHeaders(headers);
		debugf("  Elapsed: %s", timer.elapsedTime().toString().c_str());

		delete headersPtr;
	}

	void testHttpHeaders()
	{
		HttpHeaders headers;

		DEFINE_FSTR_LOCAL(FS_X_Auth_Header, "X-Auth-Header");
		DEFINE_FSTR_LOCAL(FS_x_auth_header, "x-auth-header");
		DEFINE_FSTR_LOCAL(FS_X_AUTH_HEADER, "X-AUTH-HEADER");
		DEFINE_FSTR_LOCAL(FS_valueData, "Value Data");
		DEFINE_FSTR_LOCAL(FS_valueDataNew, "Value Data New");

		headers[FS_X_Auth_Header] = FS_valueData;
		headers[FS_x_auth_header] = FS_valueDataNew;
		REQUIRE(headers.count() == 1);
		REQUIRE(headers[FS_X_AUTH_HEADER] == FS_valueDataNew);
		printHeaders(headers);

		headers.remove(FS_x_auth_header);
		REQUIRE(headers.count() == 0);
		printHeaders(headers);

		DEFINE_FSTR_LOCAL(FS_corsTest, "This is access control origin");
		DEFINE_FSTR_LOCAL(FS_acao, "access-control-Allow-Origin");
		headers[HTTP_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN] = FS_corsTest;
		REQUIRE(headers[FS_acao] == FS_corsTest);
		REQUIRE(headers.count() == 1);

		headers.clear();
		REQUIRE(headers.count() == 0);

		TEST_CASE("Non-existent (const)")
		{
			const HttpHeaders& constHeaders = headers;
			String hdr = constHeaders["Non-Existent"];
			REQUIRE(headers.count() == 0);
			REQUIRE(!hdr);
			REQUIRE(hdr.length() == 0);
			printHeaders(headers);
		}

		TEST_CASE("Non-existent (non-const)")
		{
			String hdr = headers["Non-Existent"];
			REQUIRE(headers.count() == 1);
			REQUIRE(!hdr);
			REQUIRE(hdr.length() == 0);
			printHeaders(headers);
		}

		DEFINE_FSTR_LOCAL(FS_serialized, "Non-Existent: \r\n"
										 "Mary: Had a little lamb\r\n"
										 "Content-Length: 12345\r\n"
										 "George: Was an idiot\r\n");

		auto serialize = [](HttpHeaders& h) {
			String s;
			for(unsigned i = 0; i < h.count(); ++i) {
				s += h[i];
			}
			return s;
		};

		TEST_CASE("Serialisation")
		{
			headers["Mary"] = "Had a little lamb";
			headers[HTTP_HEADER_CONTENT_LENGTH] = "12345";
			headers["George"] = "Was an idiot";
			REQUIRE(serialize(headers) == FS_serialized);
			printHeaders(headers);
		}

		TEST_CASE("setMultiple()")
		{
			HttpHeaders headers2;
			headers2.setMultiple(headers);
			REQUIRE(serialize(headers2) == FS_serialized);
			printHeaders(headers2);
		}
	}
};

void REGISTER_TEST(http)
{
	registerGroup<HttpTest>();
}
