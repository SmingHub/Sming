#include <user_config.h>
#include <SmingCore/SmingCore.h>

HttpServer server;

void Index(HttpRequest &request, HttpResponse &response)
{
	String body = "<html>\r\n";
	body += "<head>\r\n";
	body += "<title>SMING HTTP Upload example</title>\r\n";
	body += "</head>\r\n";
	body += "<body><br>\r\n";
	body += "This example illustrates the FileUpload mechanism in Sming<br>\r\n";
	body += "Please ensure that your SPIFFs Filesystem is large enough to for the uploaded data<br>\r\n";
	body += "<br><br>\r\n";
	body += "<a href=\"/list\">File List on SPIFFs</a><br>\r\n";
	body += "<br>\r\n";
	body += "<a href=\"/upload\">Simple file upload</a><br>\r\n";
	body += "<a href=\"/upload_with_vars\">File upload with extra variables</a><br>\r\n";
	body += "<a href=\"/upload_two_files\">File upload with 2 files at the same time</a><br>\r\n";
	body += "<a href=\"/custom_upload\">File upload with custom write mechanism</a><br>\r\n";
	body += "<a href=\"/upload_forbidden\">File uploading not allowed</a><br>\r\n";
	body += "</body>\r\n";
	body += "</html>\r\n";
	response.sendString(body);
}

void List(HttpRequest &request, HttpResponse &response)
{
	Vector<String> files = fileList();
	String body = "<html>\r\n";
	body += "<head>\r\n";
	body += "<title>SMING HTTP File List</title>\r\n";
	body += "</head>\r\n";
	body += "<body><br>\r\n";
	body += "File list<br>\r\n";
	body += "<hr>\r\n";


	for (int i = 0; i < files.size(); i++)
	{
		body += "<a href=\"/"+ files[i] +"\" target=\"_blank\">"+ files[i] +"</a><br>\r\n";
	}


	body += "</body>\r\n";
	body += "</html>\r\n";
	response.sendString(body);
}

void UploadPage(HttpRequest &request, HttpResponse &response)
{
	String body = "<html>\r\n";
	body += "<head>\r\n";
	body += "<title>SMING HTTP Upload example</title>\r\n";
	body += "</head>\r\n";
	body += "<body><br>\r\n";
	if(request.getRequestMethod() == RequestMethod::POST)
	{
		if(request.hasUpload()) {
			Vector<HttpUpload*> uploads = request.getUploads();
			HttpUpload* upload = uploads.get(0);
			if (upload->status == HTTP_UPLOAD_FINISHED) {
				body += "Successfully uploaded <a href=\"/"+ String(upload->filename) +"\" target=\"_blank\">"+ String(upload->filename) +" (" + String(upload->totalSize) + " bytes)</a> <br><br>\r\n";
			}
			else
			{
				body += "Upload failed for "+ String(upload->filename) +" <br>\r\n";
			}
		}
		else
		{
			body += "No uploaded data <br>\r\n";
		}

	}
	body += "<form method='POST' action='/upload' enctype='multipart/form-data'>\r\n";
	body += "<input type='file' name='file'>\r\n";
	body += "<input type='submit' value='Upload'>\r\n";
	body += "</form>\r\n";
	body += "</body>\r\n";
	body += "</html>\r\n";
	response.sendString(body);
}

void UploadPageTwoFiles(HttpRequest &request, HttpResponse &response)
{
	String body = "<html>\r\n";
	body += "<head>\r\n";
	body += "<title>SMING HTTP Upload example</title>\r\n";
	body += "</head>\r\n";
	body += "<body><br>\r\n";
	if(request.getRequestMethod() == RequestMethod::POST)
	{
		if(request.hasUpload()) {
			Vector<HttpUpload*> uploads = request.getUploads();
			for (int i = 0; i < uploads.size(); i++)
			{
				HttpUpload* upload = uploads[i];
				if (upload->status == HTTP_UPLOAD_FINISHED) {
					body += "Successfully uploaded <a href=\"/"+ String(upload->filename) +"\" target=\"_blank\">"+ String(upload->filename) +" (" + String(upload->totalSize) + " bytes)</a> <br><br>\r\n";
				}
				else
				{
					body += "Upload failed for "+ String(upload->filename) +" <br>\r\n";
				}
			}
		}
		else
		{
			body += "No uploaded data <br><br>\r\n";
		}

	}
	body += "<form method='POST' action='/upload_two_files' enctype='multipart/form-data'>\r\n";
	body += "<p><input type='file' name='file'></p>\r\n";
	body += "<p><input type='file' name='file_two'></p>\r\n";
	body += "<p><input type='submit' value='Upload'></p>\r\n";
	body += "</form>\r\n";
	body += "</body>\r\n";
	body += "</html>\r\n";
	response.sendString(body);
}


void UploadPageWithVars(HttpRequest &request, HttpResponse &response)
{
	String body = "<html>\r\n";
	body += "<head>\r\n";
	body += "<title>SMING HTTP Upload example</title>\r\n";
	body += "</head>\r\n";
	body += "<body><br>\r\n";
	if(request.getRequestMethod() == RequestMethod::POST)
	{
		if(request.hasUpload()) {
			Vector<HttpUpload*> uploads = request.getUploads();
			HttpUpload* upload = uploads.get(0);
			if (upload->status == HTTP_UPLOAD_FINISHED) {
				body += "Successfully uploaded <a href=\"/"+ String(upload->filename) +"\" target=\"_blank\">"+ String(upload->filename) +" (" + String(upload->totalSize) + " bytes)</a> <br><br>\r\n";
			}
			else
			{
				body += "Upload failed for "+ String(upload->filename) +" <br>\r\n";
			}
		}
		else
		{
			body += "No uploaded data <br><br>\r\n";
		}
		body += "post  var:  "+ request.getPostParameter("postvar") +" <br>\r\n";
		body += "query var: "+ request.getQueryParameter("query") +" <br>\r\n";


	}
	body += "<form method='POST' action='/upload_with_vars?query=myquery' enctype='multipart/form-data'>\r\n";
	body += "<p>post: <input type='text' name='postvar'></p>\r\n";
	body += "<p>File: <input type='file' name='file'></p>\r\n";
	body += "<p><input type='submit' value='Upload'></p>\r\n";
	body += "</form>\r\n";
	body += "</body>\r\n";
	body += "</html>\r\n";
	response.sendString(body);
}

void UploadHandler(HttpRequest& request, HttpUpload& upload) {
	if(upload.status == HTTP_UPLOAD_BEGIN)
	{
		debugf("Got upload request for file: %s", upload.filename.c_str());
		// we don`t need to do anything here to accept the upload
		// if we don`t want to save the contents we would set the
		// status to HTTP_UPLOAD_SKIP
		//
		// upload.status == HTTP_UPLOAD_SKIP

	}
	else if (upload.status == HTTP_UPLOAD_FINISHED)
	{
		debugf("Upload finished");
	}
}

void CustomUploadPage(HttpRequest &request, HttpResponse &response)
{
	String body = "<html>\r\n";
	body += "<head>\r\n";
	body += "<title>SMING HTTP Upload custom example</title>\r\n";
	body += "</head>\r\n";
	body += "<body><br>\r\n";
	if(request.getRequestMethod() == RequestMethod::POST)
	{
		if(request.hasUpload()) {
			Vector<HttpUpload*> uploads = request.getUploads();
			HttpUpload* upload = uploads.get(0);
			if (upload->status == HTTP_UPLOAD_FINISHED) {
				body += "Successfully uploaded <a href=\"/"+ String(upload->filename) +"\" target=\"_blank\">"+ String(upload->filename) +" (" + String(upload->totalSize) + " bytes)</a> <br><br>\r\n";
			}
			else
			{
				body += "Upload failed for "+ String(upload->filename) +" <br>\r\n";
			}
		}
		else
		{
			body += "No uploaded data <br><br>\r\n";
		}
	}
	body += "<form method='POST' action='/custom_upload' enctype='multipart/form-data'>\r\n";
	body += "<input type='file' name='file'>\r\n";
	body += "<input type='submit' value='Upload'>\r\n";
	body += "</form>\r\n";
	body += "</body>\r\n";
	body += "</html>\r\n";
	response.sendString(body);
}

void CustomUploadHandler(HttpRequest& request, HttpUpload& upload) {
	if(upload.status == HTTP_UPLOAD_BEGIN)
	{

		// to initialize the custom callback, status needs
		// to be set to HTTP_UPLOAD_WRITE_CUSTOM
		upload.file = fileOpen(upload.filename, eFO_CreateNewAlways | eFO_WriteOnly);
		upload.status = HTTP_UPLOAD_WRITE_CUSTOM;
	}
	else if(upload.status == HTTP_UPLOAD_WRITE_CUSTOM)
	{
		// called every time new data is available
		// upload.bufferdata is a point to the buffer data
		// upload.curSize is the length of the current uplod data

		int res = fileWrite(upload.file, upload.bufferdata, upload.curSize);
		if (res < 0) {
			debugf("failed writing");
			fileClose(upload.file);
			fileDelete(upload.file);

			// mark that the upload had an error and we should not
			// try to write data again
			upload.status == HTTP_UPLOAD_ERROR;
		}

	}
	else if(upload.status == HTTP_UPLOAD_FINISHED)
	{
		// called when we the end of the upload data is reached
		fileClose(upload.file);
	}
	else if(upload.status == HTTP_UPLOAD_ABORT)
	{
		// This is called when the client has a timeout
		// or aborts the upload inbetween
		fileClose(upload.file);
		fileDelete(upload.file);


	}
}

void UploadForbidden(HttpRequest &request, HttpResponse &response)
{
	String body = "<html>\r\n";
	body += "<head>\r\n";
	body += "<title>SMING HTTP Upload Forbidden example</title>\r\n";
	body += "</head>\r\n";
	body += "<body><br>\r\n";
	body += "In this example the upload will fail with a BadRequest response<br><br>\r\n";
	body += "<form method='POST' action='/upload_forbidden' enctype='multipart/form-data'>\r\n";
	body += "<input type='file' name='file'>\r\n";
	body += "<input type='submit' value='Upload'>\r\n";
	body += "</form>\r\n";
	body += "</body>\r\n";
	body += "</html>\r\n";
	response.sendString(body);
}



void onFile(HttpRequest &request, HttpResponse &response)
{
	String file = request.getPath();
	if (file[0] == '/')	file = file.substring(1);
	response.setCache(86400, true); // It's important to use cache for better performance.
	response.sendFile(file);

}

void startServer() {
		server.listen(80);
		server.addPath("/", Index);
		server.addPath("/list", List);
		server.addPath("/upload", UploadPage, UploadHandler);
		server.addPath("/upload_two_files", UploadPageTwoFiles, UploadHandler);
		server.addPath("/upload_with_vars", UploadPageWithVars, UploadHandler);
		server.addPath("/custom_upload", CustomUploadPage, CustomUploadHandler);
		server.addPath("/upload_forbidden", UploadForbidden);
		server.setDefaultHandler(onFile);
}

void init()
{
	spiffs_mount();
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Enable debug output to serial

	WifiStation.enable(false);
	WifiAccessPoint.enable(true);
	WifiAccessPoint.config("Sming Webserver", "", AUTH_OPEN);

	// Run WEB server on system ready
	System.onReady(startServer);
}
