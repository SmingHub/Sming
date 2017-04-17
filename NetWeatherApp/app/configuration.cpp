#include "../include/configuration.h"

#include <SmingCore/SmingCore.h>



//NetWeatherCfg::NetWeatherCfg()
//{
//	if (fileExist(NETWEATHER_CONFIG_FILE))
//	{
//
//		int size = fileGetSize(NETWEATHER_CONFIG_FILE);
//		char* jsonString = new char[size + 1];
//		fileGetContent(NETWEATHER_CONFIG_FILE, jsonString, size + 1);
////		cfg = jsonBuffer.parseObject(jsonString);
//
////		if (cfg->success())
////		{
//
//			// don't free jsonString as jsonBuffer refernces it..I think...
////			delete[] jsonString;
//			// Return and do not load from environmental variables
////			return;
////		}
//
//	}
//
////	NetworkSSID = WIFI_SSID;
////	NetworkPassword = WIFI_PWD;
////	cfg = &jsonBuffer.createObject();
////	cfg->["network"]["ssid"] = WIFI_SSID;
////	cfg->["network"]["password"] = WIFI_PWD;
//
//	saveCfg();
//
//	return;
//}
//
//void NetWeatherCfg::saveCfg()
//{
//
//
////	char buf[3048];
////	cfg.prettyPrintTo(buf, sizeof(buf));
////	fileSetContent(NETWEATHER_CONFIG_FILE, buf);
//
//}
//
//String NetWeatherCfg::getNetSSID(){
//	return NetworkSSID;
//}
//
//String NetWeatherCfg::getNetPassword(){
//	return NetworkPassword;
//}


