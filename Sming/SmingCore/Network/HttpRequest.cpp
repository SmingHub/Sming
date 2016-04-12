/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "HttpRequest.h"
#include "HttpServer.h"
#include "NetUtils.h"
#include <stdlib.h>
#include "../SmingCore.h"
#include "../../Services/WebHelpers/escape.h"

HttpRequest::HttpRequest()
{
	requestHeaders = NULL;
	requestGetParameters = NULL;
	requestPostParameters = NULL;
	cookies = NULL;
	headerDataProcessed = 0;
	postDataProcessed = 0;
	postParamSize = 0;
	multipart = NULL;
	uploads = NULL;
	tmpbuf = "";
}

HttpRequest::~HttpRequest()
{
	delete requestHeaders;
	delete requestGetParameters;
	delete requestPostParameters;
	delete cookies;
	headerDataProcessed = 0;
	postDataProcessed = 0;
	postParamSize = 0;

	// cleanup multipart
	if(multipart != NULL )
	{
		if (multipart->buf != NULL)
		{
			delete[] multipart-> buf;
		}
		delete multipart;
	}
	// cleanup upload objects
	if(uploads != NULL)
	{
		for (int i = 0; i < uploads->count(); i++) {
			delete uploads->get(i);
		}
		delete uploads;
	}

}

String HttpRequest::getQueryParameter(String parameterName, String defaultValue /* = "" */)
{
	if (requestGetParameters && requestGetParameters->contains(parameterName))
			return (*requestGetParameters)[parameterName];

	return defaultValue;
}
String HttpRequest::getPostParameter(String parameterName, String defaultValue /* = "" */)
{
	if (requestPostParameters && requestPostParameters->contains(parameterName))
			return (*requestPostParameters)[parameterName];

	return defaultValue;
}

String HttpRequest::getHeader(String headerName, String defaultValue /* = "" */)
{
	headerName.toLowerCase();
	if (requestHeaders && requestHeaders->contains(headerName))
		return (*requestHeaders)[headerName];

	return defaultValue;
}

String HttpRequest::getCookie(String name, String defaultValue /* = "" */)
{
	if (cookies && cookies->contains(name))
		return (*cookies)[name];

	return defaultValue;
}

int HttpRequest::getContentLength()
{
	String len = getHeader("Content-Length");
	if (len.length() == 0) return -1;

	return len.toInt();
}

String HttpRequest::getContentType()
{
	return getHeader("Content-Type");
}

HttpParseResult HttpRequest::parseHeader(HttpServer *server, pbuf* buf)
{
	int headerEnd = NetUtils::pbufFindStr(buf, "\r\n\r\n");
	if (headerEnd > NETWORK_MAX_HTTP_PARSING_LEN || headerDataProcessed > NETWORK_MAX_HTTP_PARSING_LEN \
		|| (headerEnd != -1 && buf->tot_len > NETWORK_MAX_HTTP_PARSING_LEN))
	{
		debugf("NETWORK_MAX_HTTP_PARSING_LEN");
		return eHPR_Failed_Header_Too_Large;
	}
	int urlEnd = 0;
	tmpbuf += NetUtils::pbufStrCopy(buf, 0, buf->tot_len);
	if (requestHeaders == NULL) {
		// first time calling header
		requestHeaders = new HashMap<String, String>();

		int urlStart = tmpbuf.indexOf(" ")+1;
		urlEnd = tmpbuf.indexOf(" ", urlStart);
		if (urlStart == 0 || urlEnd == -1)
		{
			debugf("!BadRequest");
			return eHPR_Failed;
		}

		method = tmpbuf.substring(0, urlStart-1);
		int urlParamsStart = tmpbuf.indexOf("?", urlStart);
		if (urlParamsStart != -1 && urlParamsStart < urlEnd)
		{
			path = tmpbuf.substring(urlStart, urlParamsStart);
			if (requestGetParameters == NULL) requestGetParameters = new HashMap<String, String>();
			extractParsingItemsList(tmpbuf, urlParamsStart + 1, urlEnd, '&', ' ', requestGetParameters);
		}
		else
			path = tmpbuf.substring(urlStart, urlEnd);
		//TODO: uri unescape path
		char* ppath = uri_unescape(NULL, 0, path.c_str(), -1);
		path = ppath;
		free(ppath);
		debugf("path=%s", path.c_str());
		urlEnd = tmpbuf.indexOf("\r\n", urlEnd)+2;
	}

	int line, nextLine;
	line = urlEnd;
	do
	{

		nextLine = tmpbuf.indexOf("\r\n", line);
		if (nextLine - line > 2)

		{
			int delim = tmpbuf.indexOf(":", line);
			if (delim != -1)
			{
				String name = tmpbuf.substring(line, delim);
				name.toLowerCase();
				if (server->isHeaderProcessingEnabled(name))
				{
					if (name == "cookie")
					{
						if (cookies == NULL) cookies = new HashMap<String, String>();
						extractParsingItemsList(tmpbuf, delim + 1, nextLine, ';', '\r', cookies);
					}
					else if(name == "content-type")
					{
						String value = tmpbuf.substring(delim + 1, nextLine);
						value.trim();
						String tmpvalue = value;
						tmpvalue.toLowerCase();
						if (tmpvalue.indexOf(ContentType::FormMultipart) != -1) {
							//is multipart header
							int boundary_start = tmpvalue.indexOf("boundary=");
							if(boundary_start == -1) {
								debugf("missing boundary param");
								return eHPR_Failed;
							}
							String boundary = extractHeaderValue(value, "=", ";", boundary_start+8);
							(*requestHeaders)["boundary"] = boundary;
							debugf("multipart - boundary === %s",  boundary.c_str());
							value = value.substring(0, value.indexOf(";"));
						}
						(*requestHeaders)[name] = value;
						debugf("%s === %s", name.c_str(), value.c_str());
					}
					else
					{
						String value = tmpbuf.substring(delim + 1, nextLine);
						value.trim();
						(*requestHeaders)[name] = value;
						debugf("%s === %s", name.c_str(), value.c_str());
					}

				}
			}

		}
		if (nextLine != -1) {
			line = nextLine + 2;
		}

	} while(nextLine != -1);

	if (headerEnd != -1)
	{
		tmpbuf = "";
		debugf("parsed");
		return eHPR_Successful;
	}
	headerDataProcessed += buf->tot_len;
	tmpbuf = tmpbuf.substring(line, buf->tot_len);
	return eHPR_Wait;
}

HttpParseResult HttpRequest::parsePostData(HttpServer *server, pbuf* buf)
{
	int start = 0;
	tmpbuf += NetUtils::pbufStrCopy(buf, 0, buf->tot_len);
	// First enter
	if (requestPostParameters == NULL)
	{
		int headerEnd = tmpbuf.indexOf("\r\n\r\n");
		if (headerEnd == -1) return eHPR_Failed;
		if (headerEnd + getContentLength() > NETWORK_MAX_HTTP_PARSING_LEN)
		{
			debugf("NETWORK_MAX_HTTP_PARSING_LEN");
			return eHPR_Failed;
		}
		requestPostParameters = new HashMap<String, String>();
		start = headerEnd + 4;
		if(buf->len - start == 0)
		{
			tmpbuf = "";
			return eHPR_Wait; // no more data
		}
		tmpbuf = tmpbuf.substring(start, tmpbuf.length());
	}

	//parse if it is FormUrlEncoded - otherwise keep in buffer
	String conType = getContentType();
	conType.toLowerCase();
	if (conType.indexOf(ContentType::FormUrlEncoded) != -1)
	{
		tmpbuf = extractParsingItemsList(tmpbuf, 0, tmpbuf.length(), '&', ' ', requestPostParameters);
	}

	postDataProcessed += buf->tot_len - start ;

	if (postDataProcessed == getContentLength())
	{
		return eHPR_Successful;
	}
	else if (postDataProcessed > getContentLength())
	{
		//avoid bufferoverflow if client announces non-correct content-length
		debugf("NETWORK_MAX_HTTP_PARSING_LEN");
		return eHPR_Failed;
	}
	else
	{
		return eHPR_Wait;
	}
}

HttpParseResult HttpRequest::parseMultipartPostData(HttpServer *server, pbuf* buf)
{
	int start = 0;

	// First enter
	if (requestPostParameters == NULL)
	{
		debugf("First Enter");

		int headerEnd = NetUtils::pbufFindStr(buf, "\r\n\r\n");
		if (headerEnd == -1) return eHPR_Failed;


		if(!server->isUploadEnabled(path)) {
			debugf("Upload not allowed for %s", path.c_str());
			return eHPR_Failed;
		}
		// multipart - create buffer and set initial status
		multipart = new Multipart();
		multipart->buflen = 0;
		multipart->buf = NULL;
		multipart->status = MULTIPART_START;
		multipart->begin_boundary = "--"+getHeader("boundary");
		multipart->end_boundary = "--"+getHeader("boundary")+"--";
		multipart->upload = NULL;

		// TODO: better method for checking free space
		u32_t total, used;
		long res = SPIFFS_info(&_filesystemStorageHandle, &total, &used);
		// check if we have enough free space
		debugf("Free space: %i - content-length: %i", total-used, getContentLength());
		if(total - used <= getContentLength()) {

			debugf("UPLOAD TOO LARGE!");
			return eHPR_Failed_Not_Enough_Space;
		}



		requestPostParameters = new HashMap<String, String>();
		start = headerEnd + 4;
		if(buf->len - start == 0) return eHPR_Wait;

	}

	String name, filename, cdisp, ctype;
	int mpheaderstart = start;
	int mpheaderend = 0;
	int curbuflen = 0;

	char* curbuf;


	if(multipart->buf ==  NULL )
	{

		// no previous buffer - just copy the data over to curbuf
		debugf("mpbuf->len == 0");
		curbuflen = buf->len - start;
		curbuf = new char[curbuflen];
		pbuf_copy_partial(buf, curbuf, curbuflen, start);

		debugf("curbuflen %i ", curbuflen);
	}
	else
	{

		// previous buffer - combine the old buffer and the pbuf data
		debugf("mpbuf->len >> 0");
		curbuflen =  buf->len - start + multipart->buflen;
		curbuf = new char[curbuflen];
		os_memcpy(curbuf, multipart->buf, multipart->buflen);
		pbuf_copy_partial(buf, &curbuf[multipart->buflen], buf->len - start, start);
		delete[] multipart->buf;
		multipart->buf = NULL;

		debugf("curbuflen %i mpbuf->len %i", curbuflen, multipart->buflen);
	}


	while(true)
	{
		if (multipart->status == MULTIPART_START)
		{

			// looking for header in multipart
			debugf("parsing multipart header");
			mpheaderend = NetUtils::cbufFindStr(curbuf, "\r\n\r\n", curbuflen, mpheaderstart);

			//debugf("mpheaderstart %i - mpheaderend %i", mpheaderstart, mpheaderend);
			if (mpheaderend == -1 )
			{

				if(curbuflen > NETWORK_MAX_HTTP_PARSING_LEN) {
					debugf("missing multipart header");
					return eHPR_Failed_Body_Too_Large;
				}
				multipart->buflen = curbuflen-mpheaderstart;
				if(multipart->buf != NULL) delete[] multipart->buf;
				multipart->buf = new char[curbuflen-mpheaderstart];
				os_memcpy(multipart->buf, &curbuf[mpheaderstart], curbuflen-mpheaderstart);
				debugf("waiting for multipart header");
				break;
			}
			String header = NetUtils::cbufStrCopy(curbuf, mpheaderstart, mpheaderend-mpheaderstart+4);
			//debugf("===== HEADER ==========");
			//debugf("%s", header.c_str());
			//debugf("=======================");

			int boundary = header.indexOf(multipart->begin_boundary.c_str());
			if(boundary == -1)
			{
				debugf("missing boundary");
				return eHPR_Failed;
			}

			ctype = "";
			cdisp = "";
			filename = "";
			name = "";
			int line, nextLine;
			line = boundary + multipart->begin_boundary.length()+2;
			do
			{

				nextLine = header.indexOf("\r\n", line);
				if (nextLine - line > 2)

				{
					int delim = header.indexOf(":", line);
					if (delim != -1)
					{
						String name = header.substring(line, delim);
						name.toLowerCase();
						String value = header.substring(delim + 1, nextLine);
						value.trim();
						debugf("%s === %s", name.c_str(), value.c_str());
						if(name == "content-disposition")
						{
							cdisp = value;
						}
						else if (name == "content-type")
						{
							ctype = value;
						}

					}

				}
				if (nextLine != -1) line = nextLine + 2;

			} while(nextLine != -1);

			if (cdisp == "")
			{
				debugf("missing ctype");
				return eHPR_Failed;
			}
			else
			{

				int n = cdisp.indexOf("name");
				if(n != -1) name = extractHeaderValue(cdisp, "=", ";", n);

				int fn = cdisp.indexOf("filename");
				if(fn != -1) filename = extractHeaderValue(cdisp, "=", ";", fn);


			}

			if(name == "") {
				debugf("missing variable name");
				return eHPR_Failed;
			}

			// HTTP RFC - if no Content-Type supplied assume text/plain
			if(ctype == "") ctype = "text/plain";

			if(filename != "")
			{
				//TODO: check that filename is not longer than 31chars
				//TODO: check that filename only has valid chars
				//if(multipart->upload != NULL) delete multipart->upload;

				multipart->upload = new HttpUpload();

				multipart->upload->filename = filename;
				multipart->upload->totalSize = 0;
				multipart->upload->name = name;
				multipart->upload->type = ctype;
				multipart->upload->status = HTTP_UPLOAD_BEGIN;
				if(uploads == NULL) uploads = new Vector<HttpUpload*>;
				uploads->add(multipart->upload);

				debugf("callback for HTTP upload start");
				server->uploads[path](*this, *multipart->upload);

				if(multipart->upload->status != HTTP_UPLOAD_WRITE_CUSTOM)
				{
					//multipart->upload->status = HTTP_UPLOAD_BEGIN; //ensure we are at begin
					handleUpload();
					multipart->upload->status = HTTP_UPLOAD_WRITE;
				}


			}
			else
			{
				// not a file upload
				//if(multipart->upload != NULL) delete multipart->upload;
				multipart->upload = NULL;
			}

			multipart->status = MULTIPART_CONTENT;
			mpheaderend += 4;

		}

		int nextboundary = NetUtils::cbufFindStr(curbuf, multipart->begin_boundary.c_str(), curbuflen, mpheaderend);

		if (nextboundary == -1) {
			debugf("no boundary");
			// if we didn`t find boundary - we need to write last packet until current
			// packetend substracing X chars and keep them in buffer to see if the
			// boundary is split between packets/pbufs

			int boundarylen = getHeader("boundary").length() + 2;
			if (curbuflen < boundarylen) boundarylen = curbuflen;
			multipart->buflen = boundarylen;
			if(multipart->buf != NULL) delete[] multipart->buf;
			multipart->buf = new char[boundarylen];
			os_memcpy(multipart->buf, &curbuf[curbuflen-boundarylen], boundarylen);

			debugf("curfbuflen %i boundarylen %i mpbuf->len ", curbuflen, boundarylen, multipart->buflen);

			if(multipart->upload == NULL)
			{
				//not a file parsing as normal post parameter
				postParamSize += curbuflen-mpheaderend;

				// check if we can actually parse it
				if (postParamSize > NETWORK_MAX_HTTP_PARSING_LEN)
				{

					debugf("NETWORK_MAX_HTTP_PARSING_LEN");
					return eHPR_Failed_Body_Too_Large;
				}

				String value = NetUtils::cbufStrCopy(curbuf, mpheaderend, curbuflen-mpheaderend-boundarylen);

				if(getPostParameter(name) == "")
				{
					(*requestPostParameters)[name] = value;
				}
				else
				{

					(*requestPostParameters)[name] += value;
				}
				debugf("requestPostParameter %s === %s", name.c_str(), getPostParameter(name).c_str());

			}
			else
			{
				if(multipart->upload->status != HTTP_UPLOAD_SKIP && multipart->upload->status != HTTP_UPLOAD_ABORT)
				{
					multipart->upload->bufferdata = &curbuf[mpheaderend];
					multipart->upload->curSize = curbuflen - mpheaderend -boundarylen;

					multipart->upload->totalSize += multipart->upload->curSize;

					//debugf("startPos %i - curSize %i", mpheaderend, multipart->upload->curSize);
					if(multipart->upload->status == HTTP_UPLOAD_WRITE_CUSTOM)
					{

						server->uploads[path](*this, *multipart->upload);
					}
					else
					{
						handleUpload();
					}
				}
			}

			//end loop since there is no more boundary in the buffer
			break;

		} else {
			debugf("found boundary");
			if(multipart->upload == NULL)
			{

				//not a file parsing as normal post parameter
				postParamSize += nextboundary - mpheaderend - 2;

				// check if we can actually parse it
				if (postParamSize > NETWORK_MAX_HTTP_PARSING_LEN)
				{

					debugf("NETWORK_MAX_HTTP_PARSING_LEN");
					return eHPR_Failed_Body_Too_Large;
				}

				String value = NetUtils::cbufStrCopy(curbuf, mpheaderend, nextboundary - mpheaderend - 2);

				if(getPostParameter(name) == "")
				{

					(*requestPostParameters)[name] = value;
				}
				else
				{

					(*requestPostParameters)[name] += value;
				}

				debugf("requestPostParameter %s === %s", name.c_str(), getPostParameter(name).c_str());

			}
			else
			{
				if(multipart->upload->status != HTTP_UPLOAD_SKIP && multipart->upload->status != HTTP_UPLOAD_ERROR)
				{
					multipart->upload->bufferdata = &curbuf[mpheaderend];
					multipart->upload->curSize = nextboundary - mpheaderend - 2;
					debugf("startPos %i - curSize %i", mpheaderend, multipart->upload->curSize);
					multipart->upload->totalSize += multipart->upload->curSize;

					if(multipart->upload->status == HTTP_UPLOAD_WRITE_CUSTOM)
					{
						//write data
						server->uploads[path](*this, *multipart->upload);

						// signal end of upload reached
						multipart->upload->status = HTTP_UPLOAD_FINISHED;
						server->uploads[path](*this, *multipart->upload);

						// set bufferdata/curSize to zero
						multipart->upload->bufferdata = NULL;
						multipart->upload->curSize = 0;

					}
					else
					{
						//write data
						handleUpload();

						// signal we reached the end of upload
						multipart->upload->status = HTTP_UPLOAD_FINISHED;
						handleUpload();

						// set bufferdata/curSize to zero
						multipart->upload->bufferdata = NULL;
						multipart->upload->curSize = 0;

					}
				}
			}
			if(curbuf[nextboundary + getHeader("boundary").length() + 1] == '-' && curbuf[nextboundary + getHeader("boundary").length() + 2] == '-')
			{
				// finished parsing, end the loop
				debugf("finished parsing multipart");
				multipart->status = MULTIPART_END;
				break;
			}
			else
			{
				// continue to next part in the current buffer
				multipart->status = MULTIPART_START;
				mpheaderstart = nextboundary;
			}
		}
	}
		// cleanup heap
	delete[] curbuf;


	postDataProcessed += buf->tot_len - start ;
	//debugf("processed %i", postDataProcessed);
	if (postDataProcessed == getContentLength())
	{
		return eHPR_Successful;
	}
	else if (postDataProcessed > getContentLength())
	{
		//avoid bufferoverflow if client announces non-correct content-length
		debugf("NETWORK_MAX_HTTP_PARSING_LEN");
		return eHPR_Failed;
	}
	else
	{
		return eHPR_Wait;
	}
}

void HttpRequest::handleUpload() {
	debugf("HttpRequest::handleUpload");
	if(multipart->upload->status == HTTP_UPLOAD_BEGIN)
	{
		debugf("starting file write");
		multipart->upload->file = fileOpen(multipart->upload->filename, eFO_CreateNewAlways | eFO_WriteOnly);

	}
	else if(multipart->upload->status == HTTP_UPLOAD_WRITE)
	{
		debugf("writing upload data");
		int res = fileWrite(multipart->upload->file, multipart->upload->bufferdata, multipart->upload->curSize);
		if (res < 0) {
			debugf("failed writing");
			// cleanup
			fileClose(multipart->upload->file);
			fileDelete(multipart->upload->file);
			fileFlush(multipart->upload->file);
			multipart->upload->status = HTTP_UPLOAD_ERROR;

		}

	}
	else if(multipart->upload->status == HTTP_UPLOAD_FINISHED)
	{
		debugf("closing file");
		fileClose(multipart->upload->file);
		fileFlush(multipart->upload->file);
	}
	else if(multipart->upload->status == HTTP_UPLOAD_ABORT)
	{
		// client aborted/timeout - cleanup
		debugf("upload abort %s", multipart->upload->filename.c_str());

		fileClose(multipart->upload->file);
		fileFlush(multipart->upload->file);
		fileDelete(multipart->upload->filename);

	}

}

void HttpRequest::cleanupMultipart(HttpServerConnection& connection) {
	debugf("HttpRequest::cleanupMultipart");
	if(multipart == NULL) return;
	if(multipart->upload == NULL) return;
	if(multipart->upload->status == HTTP_UPLOAD_WRITE_CUSTOM)
	{
		debugf("upload failed");
		multipart->upload->status = HTTP_UPLOAD_ABORT;
		connection.server->uploads[path](*this, *multipart->upload);

	}
	else if(multipart->upload->status == HTTP_UPLOAD_WRITE)
	{
		debugf("upload failed");
		multipart->upload->status = HTTP_UPLOAD_ABORT;
		handleUpload();
	}

}

String HttpRequest::extractParsingItemsList(String& buf, int startPos, int endPos, char delimChar, char endChar,
													HashMap<String, String>* resultItems)
{

	int delimItem, nextItem, startItem = startPos;
	do
	{
		delimItem = buf.indexOf("=", startItem);
		nextItem = buf.indexOf(delimChar, startItem);
		//debugf("item %i  - delim %i - next %i", startItem, delimItem, nextItem);
		if (nextItem == -1) nextItem = buf.indexOf(endChar, delimItem+1);
		if (nextItem == -1) nextItem = endPos;
		if (nextItem > endPos || delimItem == -1) nextItem = endPos;
		if (delimItem == -1) break;
		String ItemName = buf.substring(startItem, delimItem);
		String ItemValue = buf.substring(delimItem+1, nextItem);
		char* nam = uri_unescape(NULL, 0, ItemName.c_str(), -1);
		ItemName = nam;
		free(nam);
		char* val = uri_unescape(NULL, 0, ItemValue.c_str(), -1);
		ItemValue = val;
		free(val);
		ItemName.trim();
		debugf("Item: Name = %s, Size = %d, Value = %s",ItemName.c_str(),ItemValue.length(),ItemValue.substring(0,80).c_str());
		(*resultItems)[ItemName] = ItemValue;
		if (nextItem == endPos) break;
		startItem = nextItem + 1;


	} while (nextItem != -1);
	return tmpbuf.substring(startItem, nextItem);

}

String HttpRequest::extractHeaderValue(String& buf, String delimChar, String endChar, int startPos /*  = 0 */)
{
	int delim = buf.indexOf(delimChar, startPos);
	if(delim == -1) return "";
	int end = buf.indexOf(endChar, delim);
	if(end == -1) end = buf.length();
	String retVal = buf.substring(delim+1, end);
	if(retVal.charAt(0) == '"' && retVal.charAt(retVal.length()-1) == '"')
	{
		retVal = retVal.substring(1, retVal.length()-1);
	}
	return retVal;
}

String HttpRequest::getBody()
{
	return tmpbuf;
}

bool HttpRequest::isAjax()
{
	String req = getHeader("HTTP_X_REQUESTED_WITH");
	return req.equalsIgnoreCase("xmlhttprequest");
}

bool HttpRequest::isWebSocket()
{
	String req = getHeader("Upgrade");
	return req.equalsIgnoreCase("websocket");
}
