/**
* This sample shows how to send en email using SMTP.
* Note that there can be issues with mail servers rejecting emails sent using this technique.
* A free account can be created using smtp2go
*/

#pragma once

class smtpClient
{
public:
	smtpClient();
	void createClient(const String&  smtp_server, int smtp_port=25);
	byte sendEmail(const String& emailFrom, const String& emailTo, const String& subject, const String& body);
	
	void close();	// Not sure about this function - its use causes grief

	// For diagnostic purposes, allow to print info just once.
	boolean		showMeta = true;
private:
	TcpClient *	client = nullptr;
};


/*
200	(nonstandard success response, see rfc876)
211	System status, or system help reply
214	Help message
220	<domain> Service ready
221	<domain> Service closing transmission channel
250	Requested mail action okay, completed
251	User not local; will forward to <forward-path>
252	Cannot VRFY user, but will accept message and attempt delivery
354	Start mail input; end with <CRLF>.<CRLF>
421	<domain> Service not available, closing transmission channel
450	Requested mail action not taken: mailbox unavailable
451	Requested action aborted: local error in processing
452	Requested action not taken: insufficient system storage
500	Syntax error, command unrecognised
501	Syntax error in parameters or arguments
502	Command not implemented
503	Bad sequence of commands
504	Command parameter not implemented
521	<domain> does not accept mail (see rfc1846)
530	Access denied (???a Sendmailism)
550	Requested action not taken: mailbox unavailable
551	User not local; please try <forward-path>
552	Requested mail action aborted: exceeded storage allocation
553	Requested action not taken: mailbox name not allowed
554	Transaction failed
*/