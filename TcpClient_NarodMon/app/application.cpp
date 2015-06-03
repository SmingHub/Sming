/* Пример отправки данных на narodmon.ru с помощью TCP-клиента.
 * By JustACat http://esp8266.ru/forum/members/120/
 * 23.04.2015
 */
#include <user_config.h>
#include <SmingCore/SmingCore.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
	#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
	#define WIFI_PWD "PleaseEnterPass"
#endif

#define NARODM_HOST "narodmon.ru"
#define NARODM_PORT 8283


Timer procTimer; // Таймер для периодического вызова отправки данных
String mac; // Переменная для хранения mac-адреса
float t1 = -2.5; // Переменная, в которой у нас хранится температура с датчика

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
	if(sourceEvent == eTCE_Connected)
	{
		/* отправляем данные по датчикам (3 штуки)
		 * T1 = t1 (температура)
		 * H1 = 8 (влажность)
		 * P1 = 712.15 (давление)
		 *
		 * после отправки сразу закроем соединение: последний параметр = true
		 */
		client.sendString("#"+mac+"\n#T1#"+t1+"\n#H1#8\n#P1#712.15\n##", true);
	}
}

bool nmOnReceive(TcpClient& client, char *buf, int size)
{
	// debug msg
	debugf("nmOnReceive");
	debugf("%s", buf);
}

// Создаем объект narodMon класса TcpClient
TcpClient narodMon(nmOnCompleted, nmOnReadyToSend, nmOnReceive);

// эта функция будет вызываться по таймеру
void sendData()
{
	// считываем показания датчиков
	// ...
	// для отладки просто увеличиваем каждый раз значение переменной t1 на 1.39 градуса
	t1 += 1.39;

	// подключаемся к серверу narodmon
	narodMon.connect(NARODM_HOST, NARODM_PORT);
}

// Когда удачно подключились к роутеру
void connectOk()
{
	// debug msg
	debugf("I'm CONNECTED to WiFi");

	// получаем MAC-адрес нашей ESP и помещаем в переменную mac
	mac = WifiStation.getMAC();
	// в верхний регистр
	mac.toUpperCase();
	// преобразуем из XXXXXXXXXXXX в XX-XX-XX-XX-XX-XX
	for (int i = 2; i < mac.length(); i += 2)
		mac = mac.substring(0, i) + "-" + mac.substring(i++);

	debugf("mac: %s", mac.c_str());

	// вызываем по таймеру функцию sendData
	procTimer.initializeMs(6 * 60 * 1000, sendData).start(); // каждые 6 минут
	// ну и заодно сразу после запуска вызываем, чтобы не ждать 6 минут первый раз
	sendData();
}

void connectFail()
{
	// Если подключение к роутеру не удалось, выводим сообщение
	debugf("I'm NOT CONNECTED!");
	// И пробуем еще раз
	WifiStation.waitConnection(connectOk, 10, connectFail);
}

void init()
{
	// Настраиваем и включаем вывод в UART для дебага
	Serial.begin(115200);
	Serial.systemDebugOutput(true);
	Serial.println("Hello friendly world! :)");

	// Отключаем AP
	WifiAccessPoint.enable(false);

	// Настраиваем и включаем Station
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiStation.enable(true);

	/* connectOk будет вызвана, когда (если) подключимся к роутеру
	 * connectFail будет вызвана, если подключиться не получится
	 * 30 - таймаут подключения (сек)
	 */
	WifiStation.waitConnection(connectOk, 30, connectFail);
}
