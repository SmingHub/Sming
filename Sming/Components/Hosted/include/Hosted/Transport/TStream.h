#pragma once
#include <Stream.h>
#include <Delegate.h>

namespace Hosted
{
namespace Transport
{
class TStream
{
public:
	using DataHandler = Delegate<bool(Stream&)>;

	void onData(DataHandler handler)
	{
		this->handler = handler;
	}

	virtual ~TStream()
	{
	}

protected:
	DataHandler handler;
};

} // namespace Transport

} // namespace Hosted
