#include "SpiHosted.h"
#include <Data/Stream/MemoryDataStream.h>

namespace Hosted {
namespace Spi {

	void registerCommands(HostedServer& server)
	{
		// Register Command Handlers
		server.registerCommand(HostedCommand_requestSpiBeginTransaction_tag, [](HostedCommand *request, HostedCommand *response)-> int {
			if(request->payload.requestSpiBeginTransaction.has_settings) {
				SPI.begin();
			}
			else {
				SPISettings settings(request->payload.requestSpiBeginTransaction.settings.speed,
									 request->payload.requestSpiBeginTransaction.settings.byteOrder,
									 request->payload.requestSpiBeginTransaction.settings.dataMode);
				SPI.beginTransaction(settings);
			}

			return 0;
		});

		server.registerCommand(HostedCommand_requestSpiTransfer_tag, [](HostedCommand *request, HostedCommand *response)-> int {
			PbData* responseData = new PbData;
			responseData->length = 0;

			MemoryDataStream* data = (MemoryDataStream*)request->payload.requestSpiTransfer.data.arg;
			if(data != nullptr) {
				size_t available = data->available();
				uint8_t* buffer = new uint8_t[available];
				size_t length = data->readBytes((char *)&buffer, available);
				SPI.transfer(buffer, length);
				responseData->value = buffer;
				responseData->length = length;

				delete data;
			}

			response->payload.responseSpiTransfer.data.funcs.encode = &pbEncodeData;
			response->payload.responseSpiTransfer.data.arg = (void* )responseData;

			return 0;
		});
	}
}}
