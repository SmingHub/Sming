#include <SmingCore.h>
#include <hostlib/CommandLine.h>
#include <hostlib/Streams.h>

auto& output = Host::standardOutput;

namespace
{
namespace Command
{
DEFINE_FSTR(testWebConstants, "testWebConstants")
}

void testWebConstants()
{
	output << __FUNCTION__ << endl;

	const char* extensions[] = {
		"htm", "html", "txt", "js",   "css", "xml", "json", "jpg", "gif",
		"png", "svg",  "ico", "gzip", "zip", "git", "sh",   "bin", "exe",
	};

	for(auto ext : extensions) {
		String contentType = ContentType::fromFileExtension(ext);
		if(!contentType) {
			contentType = F("(NOT FOUND)");
		}
		MimeType mimeType = ContentType::fromString(contentType);
		output << "  " << ext << ": " << contentType << " (#" << unsigned(mimeType) << ')' << endl;
	}
}

} // namespace

void init()
{
	/*
	 * In a real utility we'd probably need to parse command-line arguments.
	 * See Arch/Host/Components/hostlib/options.h for one way to do this.
	 */
	output << endl << "** Basic Host utility sample **" << endl;

	auto parameters = commandLine.getParameters();
	if(parameters.count() == 0) {
		output << "No command line parameters" << endl
			   << "Try doing this:" << endl
			   << "  make run HOST_PARAMETERS='command=" << String(Command::testWebConstants) << "'" << endl;
	} else {
		output << "Command-line parameters: " << parameters.count() << endl;
		for(auto& param : parameters) {
			output << param.text << endl;
			output << "     name  = '" << param.getName() << "'" << endl;
			output << "     value = '" << param.getValue() << "'" << endl;
		}
		output.println();

		auto param = parameters.findIgnoreCase("command");
		if(param) {
			if(Command::testWebConstants.equalsIgnoreCase(param.getValue())) {
				testWebConstants();
			} else {
				output << "Command '" << param.getValue() << "' not recognised. Try '"
					   << String(Command::testWebConstants) << "'." << endl;
			}
		}
	}

	output.println();

	System.restart();
}
