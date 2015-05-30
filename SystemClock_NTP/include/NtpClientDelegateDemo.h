#ifndef INCLUDE_NTPCLIENTDELEGATEDEMO_H_
#define INCLUDE_NTPCLIENTDELEGATEDEMO_H_


class ntpClientDemo
{
public:
	ntpClientDemo()
	{
		ntpResultCB = NtpTimeResultDelegate(&ntpClientDemo::ntpResult, this);
		ntpcp = new NtpClient("pool.ntp.org", 30, ntpResultCB);

		// 			Alternative way without private member variable
		//	 		ntpcp = new NtpClient("pool.ntp.org",30, NtpTimeResultDelegate (&ntpClientDemo::ntpResult, this));

	};
	~ntpClientDemo() {};

	NtpClient *ntpcp;

	void ntpResult(NtpClient& client, time_t ntpTime)
	{
		SystemClock.setTime(ntpTime, eSCUtc);
		Serial.print("ntpClientDemo Callback Time_t = ");
		Serial.print(ntpTime);
		Serial.print(" Time = ");
		Serial.println(SystemClock.getSystemTimeString());

	}

	NtpTimeResultDelegate ntpResultCB = NtpTimeResultDelegate(&ntpClientDemo::ntpResult, this);

};




#endif /* INCLUDE_NTPCLIENTDELEGATEDEMO_H_ */
