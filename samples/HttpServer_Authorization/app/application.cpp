#include <user_config.h>
#include <SmingCore/SmingCore.h>

HttpServer server;

void onIndex(HttpRequest &request, HttpResponse &response)
{
	String body = "<html>\r\n";
	body += "<head>\r\n";
	body += "<title></title>\r\n";
	body += "</head>\r\n";
	body += "<body><br>\r\n";
	body += "Credentials:<br>\r\n";
	body += "username: user<br>\r\n";
	body += "password: pass<br>\r\n";
	body += "<br><br>\r\n";
	body += "<a href=\"/user\">Authorization with just username</a><br>\r\n";
	body += "<a href=\"/pass\">Authorization with just password</a><br>\r\n";
	body += "<a href=\"/user_pass\">Authorization with username and password</a><br>\r\n";
	body += "<a href=\"/custom\">Authorization with custom response (username & password required)</a><br>\r\n";
	body += "</body>\r\n";
	body += "</html>\r\n";
	response.sendString(body);
}

/**
 * Example requiring username and password for successfull authentication
 */
void onUsernamePassword(HttpRequest &request, HttpResponse &response)
{
	if(!request.isAuthenticated("user", "pass"))
	{
		response.authorizationRequired();
		return;
	}
	response.sendString("successful authentication with username and password");
}

/**
 * Requiring only password for authentication
 */
void onPassword(HttpRequest &request, HttpResponse &response)
{
	if(!request.isAuthenticated("", "pass"))
	{
		response.authorizationRequired();
		return;
	}
	response.sendString("successful authentication with password");
}

/**
 * Requiring only username for authentication
 */
void onUsername(HttpRequest &request, HttpResponse &response)
{
	if(!request.isAuthenticated("user", "")) {
		response.authorizationRequired();
		return;
	}
	response.sendString("successful authentication with username");
}

/**
 * Example for creating a custom response when authorization fails
 */
void onCustomHandler(HttpRequest &request, HttpResponse &response)
{
	if(!request.isAuthenticated("user", "pass")) {
		response.setStatus(HttpStatusCode::Unauthorized);
		response.setHeader("WWW-Authenticate","Basic realm=\"Custom\"");
		response.setHeader("401 Wrong credentials","Wrong credentials");
		response.setHeader("Connection","close");
	} else {
		response.sendString("successful authentication with custom handler");
	}

}

void onFile(HttpRequest &request, HttpResponse &response)
{
	response.notFound();
}

void startServer() {
		server.listen(80);
		server.addPath("/", onIndex);
		server.addPath("/user", onUsername);
		server.addPath("/pass", onPassword);
		server.addPath("/user_pass", onUsernamePassword);
		server.addPath("/custom", onCustomHandler);
		server.setDefaultHandler(onFile);
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Enable debug output to serial

	WifiAccessPoint.enable(true);
	WifiAccessPoint.config("Sming Webserver", "", AUTH_OPEN);

	// Run WEB server on system ready
	System.onReady(startServer);
}
