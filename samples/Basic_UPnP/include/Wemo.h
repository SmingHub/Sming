#pragma once

#include <Network/UPnP/RootDevice.h>
#include <Network/UPnP/Enumerator.h>
#include <Data/Stream/FlashMemoryStream.h>

DECLARE_FSTR(WEMO_SERVICE_SCPD);
DECLARE_FSTR(WEMO_METAINFO_SCPD);

namespace Wemo
{
using namespace UPnP;
class Controllee;

class Socket : public Item
{
};

class SocketEnumerator //: public Enumerator<Socket, SocketEnumerator>
{
};

class WemoService : public Service
{
public:
	Controllee* device() const
	{
		return reinterpret_cast<Controllee*>(Service::device());
	}
};

class BasicEventService : public WemoService
{
public:
	String getField(Field desc) override;

	IDataSourceStream* createDescription() override
	{
		return new FlashMemoryStream(WEMO_SERVICE_SCPD);
	}

	void handleAction(ActionInfo& info) override;
};

class MetaInfoService : public WemoService
{
public:
	String getField(Field desc) override;

	IDataSourceStream* createDescription() override
	{
		return new FlashMemoryStream(WEMO_METAINFO_SCPD);
	}

	void handleAction(ActionInfo& info) override;
};

class Controllee : public RootDevice
{
public:
	using StateChangeDelegate = Delegate<void(Controllee& device)>;

	Controllee(unsigned id, const String& name) : id_(id), name_(name)
	{
		addService(&eventService);
		addService(&metaInfoService);
	}

	void onStateChange(StateChangeDelegate delegate)
	{
		stateChangeDelegate = delegate;
	}

	unsigned id() const
	{
		return id_;
	}

	virtual bool getState() const
	{
		return state_;
	}

	virtual void setState(bool state)
	{
		state_ = state;
		if(stateChangeDelegate) {
			stateChangeDelegate(*this);
		}
	}

	String getField(Field desc) override;
	bool formatMessage(Message& msg, MessageSpec& ms) override;

private:
	BasicEventService eventService;
	MetaInfoService metaInfoService;
	unsigned id_ = 0;
	String name_;
	bool state_ = false;
	StateChangeDelegate stateChangeDelegate;
};

} // namespace Wemo
