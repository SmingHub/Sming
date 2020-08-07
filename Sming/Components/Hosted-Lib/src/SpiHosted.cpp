#include <SPI.h>
#include <HostedClient.h>
#include <Data/Stream/MemoryDataStream.h>

extern HostedClient* hostedClient;

// define the static singleton
SPIClass SPI;

void SPIClass::begin()
{
	NEW_HD_COMMAND(message, SpiBeginTransaction, {
			command->has_settings = 0;
	});

	hostedClient->send(&message);
}

void SPIClass::beginTransaction(SPISettings mySettings)
{
	NEW_HD_COMMAND(message, SpiBeginTransaction, {
			command->has_settings = 1;
			command->settings.speed = mySettings.speed;
			command->settings.byteOrder = (SpiSettings_ByteOrder)mySettings.byteOrder;
			command->settings.dataMode = (SpiSettings_DataMode)mySettings.dataMode;
	});

	hostedClient->send(&message);
}

/** @brief 	transfer(uint8_t *buffer, size_t numberBytes)
 * @param	buffer in/out
 * @param	numberBytes length of buffer
 *
 * SPI transfer is based on a simultaneous send and receive:
 * The buffered transfers does split up the conversation internaly into 64 byte blocks.
 * The received data is stored in the buffer passed by reference.
 * (the data past in is replaced with the data received).
 *
 * 		SPI.transfer(buffer, size)				: memory buffer of length size
 */
void SPIClass::transfer(uint8_t* buffer, size_t numberBytes)
{
	PbData data;
	data.value = buffer;
	data.length = numberBytes;

	NEW_HD_COMMAND(message, SpiTransfer, {
			command->data.arg = (void*)&data;
			command->data.funcs.encode = &pbEncodeData;
	});

	hostedClient->send(&message);
	HostedCommand response = hostedClient->wait();

	MemoryDataStream* resultData = (MemoryDataStream*)response.payload.responseSpiTransfer.data.arg;
	if(resultData == nullptr) {
		memset(buffer, 0, numberBytes);
		return;
	}

	resultData->readBytes((char *)buffer, numberBytes);
	delete resultData;
}


uint32_t SPIClass::transfer32(uint32_t val, uint8_t bits)
{
	// TODO:
	return 0;
}
