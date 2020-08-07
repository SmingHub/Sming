#include "DigitalHosted.h"

namespace Hosted {
namespace Digital {

	void registerCommands(HostedServer& server)
	{
		// Register Command Handlers
		server.registerCommand(HostedCommand_requestPinMode_tag, [](HostedCommand *request, HostedCommand *response)-> int {
			pinMode((uint16_t)request->payload.requestPinMode.pin, (uint8_t)request->payload.requestPinMode.mode);
			return 0;
		});

		server.registerCommand(HostedCommand_requestDigitalWrite_tag, [](HostedCommand *request, HostedCommand *response)-> int {
			digitalWrite((uint16_t)request->payload.requestDigitalWrite.pin, (uint8_t)request->payload.requestDigitalWrite.value);
			return 0;
		});

		server.registerCommand(HostedCommand_requestDigitalRead_tag, [](HostedCommand *request, HostedCommand *response)-> int {
			uint8_t result = digitalRead((uint16_t)request->payload.requestDigitalRead.pin);
			response->which_payload = HostedCommand_responseDigitalRead_tag;
			response->payload.responseDigitalRead.value = result;

			return 0;
		});
	}
}}
