#pragma once

#include <Network/NtpClient.h>
#include <SystemClock.h>
#include <Timezone.h>

class NtpClientDemo
{
public:
	NtpClientDemo() : client(nullptr, 30, NtpTimeResultDelegate(&NtpClientDemo::ntpResult, this))
	{
	}

	void ntpResult(NtpClient& client, time_t ntpTime);

private:
	NtpClient client;
};
