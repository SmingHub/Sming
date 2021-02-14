#include "SpiHosted.h"
#include <Data/Stream/MemoryDataStream.h>

namespace Hosted
{
namespace Spi
{
void registerCommands(HostedServer& server)
{
	// Register Command Handlers
	server.registerCommand(HostedCommand_requestSpiBeginTransaction_tag,
						   [](HostedCommand* request, HostedCommand* response) -> int {
							   auto& r = request->payload.requestSpiBeginTransaction;
							   if(r.has_settings) {
								   SPI.begin();
							   } else {
								   auto& rs = r.settings;
								   SPISettings settings(rs.speed, rs.byteOrder, rs.dataMode);
								   SPI.beginTransaction(settings);
							   }

							   return 0;
						   });

	server.registerCommand(HostedCommand_requestSpiTransfer_tag,
						   [](HostedCommand* request, HostedCommand* response) -> int {
							   auto responseData = new PbData{};

							   auto data = static_cast<MemoryDataStream*>(request->payload.requestSpiTransfer.data.arg);
							   if(data != nullptr) {
								   size_t available = data->available();
								   auto buffer = new uint8_t[available];
								   size_t length = data->readBytes(reinterpret_cast<char*>(&buffer), available);
								   SPI.transfer(buffer, length);
								   responseData->value = buffer;
								   responseData->length = length;

								   delete data;
							   }

							   auto& data = response->payload.responseSpiTransfer.data;
							   data.funcs.encode = &pbEncodeData;
							   data.arg = responseData;

							   return 0;
						   });
}
} // namespace Spi
} // namespace Hosted
