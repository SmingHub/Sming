#include "DigitalHosted.h"

namespace Hosted
{
namespace Digital
{
void registerCommands(HostedServer& server)
{
	// Register Command Handlers
	server.registerCommand(HostedCommand_requestPinMode_tag,
						   [](HostedCommand* request, HostedCommand* response) -> int {
							   auto& r = request->payload.requestPinMode;
							   pinMode(r.pin, r.mode);
							   return 0;
						   });

	server.registerCommand(HostedCommand_requestDigitalWrite_tag,
						   [](HostedCommand* request, HostedCommand* response) -> int {
							   auto& r = request->payload.requestDigitalWrite;
							   digitalWrite(r.pin, r.value);
							   return 0;
						   });

	server.registerCommand(HostedCommand_requestDigitalRead_tag,
						   [](HostedCommand* request, HostedCommand* response) -> int {
							   auto& r = request->payload.requestDigitalRead;
							   uint8_t result = digitalRead(r.pin);
							   response->which_payload = HostedCommand_responseDigitalRead_tag;
							   response->payload.responseDigitalRead.value = result;

							   return 0;
						   });
}
} // namespace Digital
} // namespace Hosted
