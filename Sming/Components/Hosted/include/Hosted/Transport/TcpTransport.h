#include <Network/TcpServer.h>
#include "BaseTransport.h"

namespace Hosted
{
namespace Transport
{
class TcpTransport : public BaseTransport
{
protected:
	virtual bool process(TcpClient& client, char* data, int size) = 0;
};

} // namespace Transport

} // namespace Hosted
