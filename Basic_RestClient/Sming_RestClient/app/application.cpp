// Please configure your SSID, passwords and OW key on the user_config.h file
#include <user_config.h>

#include <SmingCore/SmingCore.h>
#include <Libraries/LiquidCrystal/LiquidCrystal_I2C.h>

// For more information visit useful wiki page: http://arduino-info.wikispaces.com/LCD-Blue-I2C
#define I2C_LCD_ADDR 0x27
LiquidCrystal_I2C lcd(I2C_LCD_ADDR, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

// URL of the test REST server:

#define URL "http://192.168.1.17:3003/api/test"

HttpClient httpServer;
Timer      checkTimer;  // To run tests in cycle
int        idTest = 0;  // 0: GET 1:POST  2:PUT  3:DELETE 

// HTTP request callback.
void onDataReceived(HttpClient& client, bool successful) {
    lcd.setCursor(0,1);
    if (successful) {
        Serial.println("Success: Request to Rest Server worked!");
        lcd.print("OK!");
    } else {
        Serial.println("Failure: Request failed..");
        lcd.print("NOT OK :(");
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
    lcd.clear();
    lcd.setCursor(0,0);

    if ( idTest > 3 ) idTest = 0;
    httpServer.reset(); // Clear any data from previous requests.
    httpServer.setRequestContentType("application/json");
    
    switch ( idTest) {
        case 0: lcd.print("Testing GET...");
                Serial.println("Testing GET");
                httpServer.setPostBody("{\"body\":\"for GET\"}"); // A zero lenght body will trigger a GET request.
                httpServer.doGet(URL, onDataReceived);
                break;
        case 1: lcd.print("Testing POST...");
                Serial.println("Testing POST");
                httpServer.setPostBody("{\"body\":\"for POST\"}"); // A zero lenght body will trigger a GET request.
                httpServer.doPost(URL, onDataReceived);
                break;
        case 2: lcd.print("Testing PUT...");
                Serial.println("Testing PUT");
                httpServer.setPostBody("{\"body\":\"for PUT\"}"); // A zero lenght body will trigger a GET request.
                httpServer.doPut(URL, onDataReceived);
                break;
        case 3: lcd.print("Testing DELETE...");
                Serial.println("Testing DELETE");
                httpServer.setPostBody("{\"body\":\"for DELETE\"}"); // A zero lenght body will trigger a GET request.
                httpServer.doDelete(URL, onDataReceived);
                break;
    }
               
    idTest++;
}

// Will be called when WiFi station is connected to AP
void connectOk() {
    Serial.println("I'm CONNECTED");

    ShowInfo();

    // Let's get the Mac address, that should be unique...
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
    
    lcd.begin(16, 2);   // initialize the lcd for 16 chars 2 lines, turn on backlight

    // ------- Quick 3 blinks of backlight  -------------
    for(int i = 0; i< 3; i++)
    {
            lcd.backlight();
            delay(150);
            lcd.noBacklight();
            delay(250);
    }
    lcd.backlight(); // finish with backlight on

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
