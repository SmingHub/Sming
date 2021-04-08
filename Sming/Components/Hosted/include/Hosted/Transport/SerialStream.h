#include <HardwareSerial.h>
#include "TStream.h"

namespace Hosted
{
namespace Transport
{
class SerialStream : public TStream
{
public:
	SerialStream(HardwareSerial& stream)
	{
		stream.onDataReceived(StreamDataReceivedDelegate(&SerialStream::process, this));
	}

private:
	void process(Stream& source, char arrivedChar, uint16_t availableCharsCount)
	{
		handler(source);
	}
};

} // namespace Transport

} // namespace Hosted
