#pragma once

#include <SmingCore.h>
#include <pb_encode.h>
#include <pb_decode.h>
#include "hosted.pb.h"
#include "HostedUtils.h"

constexpr int HOSTED_OK{0};
constexpr int HOSTED_FAIL{-1};
constexpr int HOSTED_NO_MEM{-2};

typedef Delegate<int(HostedCommand* request, HostedCommand* response)> HostedCommandDelegate;

// Creates new hosted command
#define NEW_HD_COMMAND(NAME, XX, YY)                                                                                   \
	HostedCommand NAME = HostedCommand_init_zero;                                                                      \
	NAME.which_payload = HostedCommand_request##XX##_tag;                                                              \
	{                                                                                                                  \
		auto command = &NAME.payload.request##XX;                                                                      \
		YY                                                                                                             \
	}

class HostedCommon
{
public:
	virtual ~HostedCommon()
	{
	}

	virtual bool onData(const char* at, size_t length) = 0;
};
