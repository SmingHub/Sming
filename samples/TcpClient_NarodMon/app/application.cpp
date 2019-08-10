/* Пример отправки данных на narodmon.ru с помощью TCP-клиента.
 * An example of sending data to narodmon.ru using a TCP client.
 * By JustACat http://esp8266.ru/forum/members/120/
 * 23.04.2015
 *
 * 9/8/2019 mikee47 Revised with English translations
 */
#include <SmingCore.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

#define NARODM_HOST "narodmon.ru"
#define NARODM_PORT 8283

// Time between command packets, in seconds
const unsigned SENDDATA_INTERVAL = 6 * 60;

// Таймер для периодического вызова отправки данных
// Timer for a periodic call to send data
Timer procTimer;

// Переменная для хранения mac-адреса
// Variable for storing MAC address
MacAddress mac;

// Переменная, в которой у нас хранится температура с датчика
// Variable to store temperature from sensor
float t1 = -2.5;

void nmOnCompleted(TcpClient& client, bool successful)
{
	// debug msg
	debugf("nmOnCompleted");
	debugf("successful: %d", successful);
}

void nmOnReadyToSend(TcpClient& client, TcpConnectionEvent sourceEvent)
{
	// debug msg
	debugf("nmOnReadyToSend");
	debugf("sourceEvent: %d", sourceEvent);

	// в момент соединения осуществляем отправку
	if(sourceEvent == eTCE_Connected) {
		// преобразуем из XXXXXXXXXXXX в XX-XX-XX-XX-XX-XX
		String command = "#" + mac.toString() + '\n';
		/* отправляем данные по датчикам (3 штуки)
		 * Send data on 3 sensors
		 */
		// T1 = t1 (температура Temperature)
		command += "#T1#" + String(t1) + '\n';
		// H1 = 8 (влажность Humidity)
		command += "#H1#8\n";
		// P1 = 712.15 (давление Pressure)
		command += "#P1#712.15\n";
		// Terminate command list
		command += "##";

		Serial.println(command);

		/*
		 * после отправки сразу закроем соединение: последний параметр = true
		 * After sending, close the connection.
		 */
		bool forceCloseAfterSent = true;
		client.sendString(command, forceCloseAfterSent);
	}
}

bool nmOnReceive(TcpClient& client, char* buf, int size)
{
	// debug msg
	debugf("nmOnReceive");
	debugf("%s", buf);
	return true;
}

// Создаем объект narodMon класса TcpClient
// Create the narodMon object
TcpClient narodMon(nmOnCompleted, nmOnReadyToSend, nmOnReceive);

// эта функция будет вызываться по таймеру
// This function is called by timer
void sendData()
{
	// Read sensors
	// считываем показания датчиков
	// ...
	// For debugging, increase the value of the variable t1 by 1.39 degrees each time
	// для отладки просто увеличиваем каждый раз значение переменной t1 на 1.39 градуса
	t1 += 1.39;

	// Connect to narodmon server
	// подключаемся к серверу narodmon
	narodMon.connect(NARODM_HOST, NARODM_PORT);
}

// Successful connection to AP
// Когда удачно подключились к роутеру
void connectOk(const String& SSID, MacAddress bssid, uint8_t channel)
{
	// debug msg
	debugf("I'm CONNECTED to WiFi");

	// Get the MAC address of our ESP
	// получаем MAC-адрес нашей ESP и помещаем в переменную mac
	mac = WifiStation.getMacAddress();
	debugf("mac: %s", mac.toString().c_str());
}

void connectFail(const String& ssid, MacAddress bssid, WifiDisconnectReason reason)
{
	// Display a message on failed connection
	// Если подключение к роутеру не удалось, выводим сообщение
	debugf("I'm NOT CONNECTED!");
}

void gotIP(IpAddress ip, IpAddress netmask, IpAddress gateway)
{
	// Call the sendData function by timer, every 6 minutes
	// вызываем по таймеру функцию sendData
	procTimer.initializeMs(SENDDATA_INTERVAL * 1000, sendData).start(); // каждые 6 минут

	// Send immediately on startup
	// ну и заодно сразу после запуска вызываем, чтобы не ждать 6 минут первый раз
	sendData();
}

void init()
{
	// Configure and enable output in UART for debug
	// Настраиваем и включаем вывод в UART для дебага
	Serial.begin(COM_SPEED_SERIAL);
	Serial.systemDebugOutput(true);
	Serial.println("Hello friendly world! :)");

	// Disable AP
	// Отключаем AP
	WifiAccessPoint.enable(false);

	// Configure and enable Station
	// Настраиваем и включаем Station
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiStation.enable(true);

	/*
	 * connectOk will be called on connection to the router
	 * connectFail will be called if it fails to connect
	 * Timeout is 30 seconds
	 */
	/* connectOk будет вызвана, когда (если) подключимся к роутеру
	 * connectFail будет вызвана, если подключиться не получится
	 * 30 - таймаут подключения (сек)
	 */
	WifiEvents.onStationConnect(connectOk);
	WifiEvents.onStationDisconnect(connectFail);
	WifiEvents.onStationGotIP(gotIP);
}
