#include <HardwareSerial.h>
#include "BaseTransport.h"

namespace Hosted
{
namespace Transport
{
class SerialTransport : public BaseTransport
{
public:
	SerialTransport(HardwareSerial& stream)
	{
		stream.onDataReceived(StreamDataReceivedDelegate(&SerialTransport::process, this));
	}

private:
	void process(Stream& source, char arrivedChar, uint16_t availableCharsCount)
	{
		handler(source);
	}
};

} // namespace Transport

} // namespace Hosted
