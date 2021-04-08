#include <Network/TcpClient.h>
#include "TcpStream.h"

namespace Hosted
{
namespace Transport
{
class TcpClientStream : public TcpStream
{
public:
	TcpClientStream(TcpClient& client)
	{
		client.setReceiveDelegate(TcpClientDataDelegate(&TcpClientStream::process, this));
		stream = new CompoundTcpClientStream(client);
	}

	~TcpClientStream()
	{
		delete stream;
	}

protected:
	bool process(TcpClient& client, char* data, int size) override
	{
		if(!stream.push(data, size)) {
			return false;
		}

		return handler(*stream);
	}

private:
	CompoundTcpClientStream* stream = nullptr;
};

} // namespace Transport

} // namespace Hosted
