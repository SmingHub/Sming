// Please configure your SSID, passwords and OW key on the user_config.h file
#include <user_config.h>

#include <SmingCore/SmingCore.h>

#define URL "http://192.168.1.17:3003/api/test"

HttpClient httpServer;
Timer      checkTimer;  // To run tests in cycle
int        idTest = 0;  // 0: GET 1:POST  2:PUT  3:DELETE 

// HTTP request callback.
void onDataReceived(HttpClient& client, bool successful) {
    if (successful) {
        Serial.println("Success: Request to Rest Server worked!");
    } else {
        Serial.println("Failure: Request failed..");
    }
}

void ShowInfo() {
    Serial.printf("\r\nSDK: v%s\r\n", system_get_sdk_version());
    Serial.printf("Free Heap: %d\r\n", system_get_free_heap_size());
    Serial.printf("CPU Frequency: %d MHz\r\n", system_get_cpu_freq());
    Serial.printf("System Chip ID: 0x%x\r\n", system_get_chip_id());
    Serial.printf("SPI Flash ID: 0x%x\r\n", spi_flash_get_id());
    Serial.printf("SPI Flash Size: %d\r\n", (1 << ((spi_flash_get_id() >> 16) & 0xff)));
}

void startMain() {

    if ( idTest > 3 ) idTest = 0;
    httpServer.reset(); // Clear any data from previous requests.
    httpServer.setRequestContentType("application/json");
    
    switch ( idTest) {
        case 0: Serial.println("Testing GET");
                httpServer.setPostBody("{\"body\":\"for GET\"}"); // Per si a Post body on a get request doesn't make sense.
                httpServer.doGet(URL, onDataReceived);
                break;
        case 1: Serial.println("Testing POST");
                httpServer.setPostBody("{\"body\":\"for POST\"}"); 
                httpServer.doPost(URL, onDataReceived);
                break;
        case 2: Serial.println("Testing PUT");
                httpServer.setPostBody("{\"body\":\"for PUT\"}"); 
                httpServer.doPut(URL, onDataReceived);
                break;
        case 3: Serial.println("Testing DELETE");
                httpServer.setPostBody("{\"body\":\"for DELETE\"}"); 
                httpServer.doDelete(URL, onDataReceived);
                break;
    }
               
    idTest++;
}

// Will be called when WiFi station is connected to AP
void connectOk() {
    Serial.println("I'm CONNECTED");

    ShowInfo();

    // Let's get the IP address
    Serial.println("Device ID: " + String(WifiStation.getIP()));

    checkTimer.initializeMs(5 * 1000, startMain).start();
}

// Will be called when WiFi station timeout was reached

void connectFail() {
    Serial.println("I'm NOT CONNECTED. Need help :(");

    WifiStation.waitConnection(connectOk, 20, connectFail); // We recommend 20+ seconds for connection timeout at start
}

void sysReady() {

    Serial.println("System ready callback called....");
}    

// Standard init function.
void init() {

    system_set_os_print(0);

    Serial.begin(SERIAL_BAUD_RATE);  // 115200 by default
    Serial.systemDebugOutput(false); // Disable debug output to serial

    // Connect to WIFI
    WifiStation.config(WIFI_SSID, WIFI_PWD);
    WifiStation.enable(true);
    WifiAccessPoint.enable(false);

    // Run our method when station was connected to AP (or not connected)
    WifiStation.waitConnection(connectOk, 20, connectFail); // We recommend 20+ seconds for connection timeout at start

    System.onReady(sysReady);      
}
