#include <SmingCore.h>
#ifdef ARCH_HOST
#include <hostlib/CommandLine.h>
#endif

#ifdef ARCH_HOST

namespace
{
#define XX(name, ext, mime) ext "\0"
DEFINE_FSTR_LOCAL(fstr_ext, "htm\0" MIME_TYPE_MAP(XX))
#undef XX

namespace Command
{
DEFINE_FSTR(testWebConstants, "testWebConstants")
}

void testWebConstants()
{
	m_printf("%s\n", __FUNCTION__);

	const char* extensions[] = {
		"htm", "html", "txt", "js",   "css", "xml", "json", "jpg", "gif",
		"png", "svg",  "ico", "gzip", "zip", "git", "sh",   "bin", "exe",
	};

	for(unsigned i = 0; i < ARRAY_SIZE(extensions); ++i) {
		auto ext = extensions[i];
		String contentType = ContentType::fromFileExtension(ext);
		if(!contentType) {
			contentType = "(NOT FOUND)";
		}
		MimeType mimeType = ContentType::fromString(contentType);
		m_printf("  %u %s: %s (#%u)\n", i, ext, contentType.c_str(), mimeType);
	}
}

} // namespace

#endif

void init()
{
	// Hook up debug output
	Serial.begin(COM_SPEED_SERIAL);
	Serial.systemDebugOutput(true);

	/*
	 * In a real utility we'd probably need to parse command-line arguments.
	 * See Arch/Host/Components/hostlib/options.h for one way to do this.
	 */
	m_printf("\n** Basic Host utility sample **\n");

#ifdef ARCH_HOST
	auto parameters = commandLine.getParameters();
	if(parameters.count() == 0) {
		m_printf("No command line parameters\n"
				 "Try doing this:\n"
				 "  make run HOST_PARAMETERS='command=%s'\n",
				 String(Command::testWebConstants).c_str());
	} else {
		m_printf("Command-line parameters:\n", parameters.count());
		for(int i = 0; i < parameters.count(); ++i) {
			auto param = parameters[i];
			m_printf("  %u: text =  '%s'\n", i, param.text);
			m_printf("     name  = '%s'\n", param.getName().c_str());
			m_printf("     value = '%s'\n", param.getValue().c_str());
		}
		m_putc('\n');

		auto param = parameters.findIgnoreCase("command");
		if(param) {
			if(Command::testWebConstants.equalsIgnoreCase(param.getValue())) {
				testWebConstants();
			} else {
				m_printf("Command '%s' not recognised. Try '%s'.\n", param.getValue().c_str(),
						 String(Command::testWebConstants).c_str());
			}
		}
	}

	m_putc('\n');

	System.restart();

#else

	Serial.println();
	Serial.println(_F("** THIS IS A HOST-ONLY APPLICATION **"));
	Serial.println();
	Serial.println(_F("Please build with SMING_ARCH=Host"));
	Serial.println();

#endif
}
