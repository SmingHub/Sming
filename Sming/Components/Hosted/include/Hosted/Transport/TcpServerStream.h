#include <Network/TcpServer.h>
#include <Data/ObjectMap.h>
#include "TcpStream.h"

namespace Hosted
{
namespace Transport
{
class TcpServerStream : public TcpStream
{
public:
	using ClientMap = ObjectMap<uint32_t, CompoundTcpClientStream>;

	TcpServerStream(TcpServer& server)
	{
		server.setClientReceiveHandler(TcpClientDataDelegate(&TcpServerStream::process, this));
	}

protected:
	bool process(TcpClient& client, char* data, int size) override
	{
		uint32_t key = uint32_t(&client);

		CompoundTcpClientStream* stream = nullptr;

		int i = map.indexOf(key);
		if(i >= 0) {
			stream = map.valueAt(i);
		} else {
			map[key] = new CompoundTcpClientStream(client);
		}

		if(!stream->push(reinterpret_cast<const uint8_t*>(data), size)) {
			return false;
		}

		return handler(*stream);
	}

private:
	ClientMap map;
};

} // namespace Transport

} // namespace Hosted
