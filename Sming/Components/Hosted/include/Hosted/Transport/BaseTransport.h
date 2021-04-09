#pragma once
#include <Stream.h>
#include <Delegate.h>

namespace Hosted
{
namespace Transport
{
class BaseTransport
{
public:
	using DataHandler = Delegate<bool(Stream&)>;

	void onData(DataHandler handler)
	{
		this->handler = handler;
	}

	virtual ~BaseTransport()
	{
	}

protected:
	DataHandler handler;
};

} // namespace Transport

} // namespace Hosted
