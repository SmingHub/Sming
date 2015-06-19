/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "HttpServerConnection.h"

#include "HttpServer.h"
#include "TcpServer.h"
#include "../../Libraries/cWebsocket/websocket.h"

HttpServerConnection::HttpServerConnection(HttpServer *parentServer, tcp_pcb *clientTcp)
	: TcpConnection(clientTcp, true), server(parentServer), state(eHCS_Ready)
{
	TcpServer::totalConnections++;
	response.setHeader("Connection", "close");
}

HttpServerConnection::~HttpServerConnection()
{
	TcpServer::totalConnections--;
}

err_t HttpServerConnection::onReceive(pbuf *buf)
{
	if (buf == NULL)
	{
		TcpConnection::onReceive(buf);
		return ERR_OK;
	}

	/*{
	 	// split.buf.test
		pbuf* dbghack = new pbuf();
		dbghack->payload = (char*)buf->payload + 100;
		dbghack->len = buf->len - 100;
		buf->len = 100;
		buf->next = dbghack;
	}*/

	if (state == eHCS_Ready)
	{
		HttpParseResult res = request.parseHeader(server, buf);
		if (res == eHPR_Wait)
			debugf("HEADER WAIT");
		else if (res == eHPR_Failed)
		{
			debugf("HEADER FAILED");
			response.badRequest();
			sendError();
		}
		else if (res == eHPR_Successful)
		{
			debugf("Request: %s, %s", request.getRequestMethod().c_str(),
					(request.getContentLength() > 0 ? (String(request.getContentLength()) + " bytes").c_str() : "nodata"));

			String contType = request.getContentType();
			contType.toLowerCase();
			if (request.getContentLength() > 0 && contType.indexOf(ContentType::FormUrlEncoded) != -1)
				state = eHCS_ParsePostData;
			else
				state = eHCS_ParsingCompleted;
		}
	}
	else if (state == eHCS_WebSocketFrames)
	{
		server->processWebSocketFrame(buf, *this);
	}

	if (state == eHCS_ParsePostData)
	{
		HttpParseResult res = request.parsePostData(server, buf);
		if (res == eHPR_Wait)
			debugf("POST WAIT");
		else if (res == eHPR_Failed)
		{
			debugf("POST FAILED");
			response.badRequest();
			sendError();
		}
		else if (res == eHPR_Successful)
		{
			debugf("POST Parsed");
			state = eHCS_ParsingCompleted;
		}
	}

	// Fire callbacks
	TcpConnection::onReceive(buf);

	return ERR_OK;
}

void HttpServerConnection::beginSendData()
{
	if (!server->processRequest(*this, request, response))
	{
		response.notFound();
		sendError();
		return;
	}

	if (!response.hasBody() && (response.getStatusCode() < 100 || response.getStatusCode() > 399))
	{
		// Show default error message
		sendError();
		return;
	}

	debugf("response sendHeader");
	response.sendHeader(*this);

	if (request.isWebSocket())
	{
		debugf("Switched to WebSocket Protocol");
		state = eHCS_WebSocketFrames; // Stay opened
		setTimeOut(USHRT_MAX);
	}
	else
		state = eHCS_Sending;
}

void HttpServerConnection::sendError(const char* message /* = NULL*/)
{
	debugf("SEND ERROR PAGE");
	response.setContentType(ContentType::HTML);
	response.sendHeader(*this);

	writeString("<H2 color='#444'>", TCP_WRITE_FLAG_MORE);
	const char* statusName = response.getStatusName().c_str();
	writeString(message ? message : statusName, TCP_WRITE_FLAG_COPY);
	writeString("</H2>");
	state = eHCS_Sent;
}

void HttpServerConnection::onReadyToSendData(TcpConnectionEvent sourceEvent)
{
	TcpConnection::onReadyToSendData(sourceEvent);

	if (state == eHCS_ParsingCompleted)
		beginSendData();

	if (state == eHCS_Sending)
	{
		debugf("response sendBody");
		if (response.sendBody(*this))
			state = eHCS_Sent; // Completed!
	}

	if (state == eHCS_Sent)
		close();
}

void HttpServerConnection::close()
{
	if (disconnection)
	{
		disconnection(*this);
		disconnection = nullptr;
	}
	TcpConnection::close();
}

void HttpServerConnection::onError(err_t err)
{
	if (disconnection)
	{
		disconnection(*this);
		disconnection = nullptr;
	}
	TcpConnection::onError(err);
}

void HttpServerConnection::setDisconnectionHandler(HttpServerConnectionDelegate handler)
{
	disconnection = handler;
}
