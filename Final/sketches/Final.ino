#include <WiFi.h>
#include <ESP32Time.h>
#include <SPIFFS.h>
#include "FS.h"
#include "SD.h"
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
//#include <WebSocketsServer.h>
//#include <Ticker.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_ADS1X15.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for SSD1306 display connected using software SPI (default case):
#define OLED_MOSI   23
#define OLED_CLK   18
#define OLED_DC    16
#define OLED_CS    5
#define OLED_RESET 17

// Declaration for SD card module connected using software SPI (default case):
#define SD_MOSI   13
#define SD_CLK   14
#define SD_MISO    27
#define SD_CS    15


//Constants
static const BaseType_t pro_cpu = 0;
static const BaseType_t app_cpu = 1;

static SemaphoreHandle_t mutex;

static TimerHandle_t auto_reload_timer = NULL;
DynamicJsonDocument doc(1024);

char jobData[200];

const char* ssid = "DRTARUN 0186";
const char* psk = "U66[27q3";
String messageDisplay;

float readBuff = 0;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
SPIClass spi = SPIClass(HSPI);
Adafruit_ADS1115 ads;
String ip_addr_str;
AsyncWebServer server(80);

void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info) {
	Serial.println("[WiFi] Connected to AP successfully!");
	messageDisplay = "Connected";
}

void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info) {
	Serial.println("[WiFi] WiFi connected");
	Serial.println("[WiFi] IP address: ");
	Serial.println(WiFi.localIP());
	ip_addr_str = WiFi.localIP().toString();
	messageDisplay = "IPV4:" + ip_addr_str;
	
	//Set Time
	delay(1000);
	ESP32Time.begin();
}

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
	messageDisplay = "Disconnect\nReconnect";
	Serial.println("[WiFi] Disconnected from WiFi access point");
	Serial.print("[WiFi] WiFi lost connection. Reason: ");
	Serial.println(info.disconnected.reason);
	Serial.println("[WiFi] Trying to Reconnect");
	
	WiFi.disconnect(true);

	delay(20000);
	
	WiFi.begin(ssid, psk);
}

void displayTask(void* parameters)
{
	float voltage = NULL;
	
	while (true)
	{
		//Serial.println("[Display] loop");
		time_t t = time(NULL);
		struct tm *t_st;
		t_st = localtime(&t);
		
		
		
		display.clearDisplay();
		display.setCursor(0, 0);
		display.setTextSize(1);
		display.printf("%02d-%02d-%04d  %02d:%02d:%02d\n", t_st->tm_mday, 1 + t_st->tm_mon, 1900 + t_st->tm_year, t_st->tm_hour, t_st->tm_min, t_st->tm_sec);
		
		display.setTextSize(2);
		display.println(messageDisplay);
		
		
		display.setTextSize(3);
		
		display.printf("%.1fmV\n", 1000 * readBuff);
		display.display();
		
		delay(100);
	}
}

void measurementTask(void* parameters)
{
	while (true)
	{
		//Serial.println("[Measurement] loop");
		readBuff = ads.computeVolts(ads.readADC_SingleEnded(1));
		delay(50);
	}
}

void logTask(void* parameters)
{
	Serial.println("[LogTask] entering task");
	int i = 0;
	while (i < 10)
	{
		if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE)
		{
			Serial.println("[LogTask] mutex lock obtained");
			File dataFile = SD.open("/data.csv", FILE_APPEND);
			time_t t = time(NULL);
			struct tm *t_st;
			t_st = localtime(&t);
			Serial.println("[LogTask] writing SD");
			dataFile.printf("%02d-%02d-%04d,%02d:%02d:%02d,%.1f\n", t_st->tm_mday, 1 + t_st->tm_mon, 1900 + t_st->tm_year, t_st->tm_hour, t_st->tm_min, t_st->tm_sec, readBuff * 1000.0f);
			dataFile.close();
			Serial.println("[LogTask] writing SD complete");
			xSemaphoreGive(mutex);
			i++;
		}
		else
		{
			Serial.println("[LogTask] mutex lock not obtained");
			continue;
		}
		delay(1000);
	}
	Serial.println("[LogTask] deleting task");
	vTaskDelete(NULL);
}

void myTimerCallback(TimerHandle_t xTimer)
{
	if ((uint32_t)pvTimerGetTimerID(xTimer) == 1)
	{
		Serial.println("[TimerCallback] assigning task");
		xTaskCreatePinnedToCore(logTask, "LogTask", 2048 * 4, NULL, 2, NULL, app_cpu);
	}
}

bool startJob()
{
	Serial.println("[StartJob] reading config.json");
	File file = SPIFFS.open("/config.json", FILE_READ);
	
	if (!file) {
		Serial.println("[StartJob] Error opening config file for reading");
		file.close();
		Serial.println("[StartJob] exiting");
		return false;
	}
	else
	{
		if (auto_reload_timer != NULL)
		{
			Serial.println("[StartJob] deleting previous Timer");
			xTimerDelete(auto_reload_timer, 0);
		}
		
		Serial.println("[StartJob] creating Timer");
		auto_reload_timer = xTimerCreate("Auto-reload timer", 60000 / portTICK_PERIOD_MS, pdTRUE, (void*)1, myTimerCallback);
		if (auto_reload_timer == NULL)
		{
			Serial.println("[StartJob] Could not create timer");
		}
		else
		{
			Serial.println("[StartJob] starting Timer");
			xTimerReset(auto_reload_timer, portMAX_DELAY);
		}
		
		while (file.available())
		{
			Serial.write(file.read());
		}
		file.close();
		Serial.println("[StartJob] exiting");
		return true;
	}
	
}

void listDir(fs::FS &fs, const char * dirname, uint8_t levels) {
	Serial.printf("[SD] Listing directory: %s\r\n", dirname);

	File root = fs.open(dirname);
	if (!root) {
		Serial.println("[SD] Failed to open directory");
		return;
	}
	if (!root.isDirectory()) {
		Serial.println("[SD] Not a directory");
		return;
	}

	File file = root.openNextFile();
	while (file) {
		if (file.isDirectory()) {
			Serial.print("[SD]   DIR : ");
			Serial.println(file.name());
			if (levels) {
				listDir(fs, file.name(), levels - 1);
			}
		}
		else {
			Serial.print("[SD]   FILE: ");
			Serial.print(file.name());
			Serial.print("[SD]   SIZE: ");
			Serial.println(file.size());
		}
		file = root.openNextFile();
	}
}

void setup()
{
	//init LED
	pinMode(LED_BUILTIN, OUTPUT);
	
	//Begin serial Communication
	Serial.begin(115200);
	
	// The ADC input range (or gain) can be changed via the following
	// functions, but be careful never to exceed VDD +0.3V max, or to
	// exceed the upper and lower limits if you adjust the input range!
	// Setting these values incorrectly may destroy your ADC!
	//                                                                ADS1015  ADS1115
	//                                                                -------  -------
	// ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
	// ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
	// ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
	// ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
	// ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
	// ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV
	ads.setGain(GAIN_TWO); 
	
	if (!ads.begin()) {
		Serial.println("[Main] Failed to initialize ADS.");
		while (true) ; //TODO: sleep
	}
	
	
	// SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
	if (!display.begin(SSD1306_SWITCHCAPVCC)) {
		Serial.println(F("[Main] SSD1306 allocation failed"));
		while (true) ; // TODO: sleep
	}
	
	
	display.clearDisplay();
	display.setTextSize(2); // Draw 2X-scale text
	display.setTextColor(SSD1306_WHITE);
	
	
	//init SPIFFS
	if (!SPIFFS.begin())
	{
		Serial.println("[Main] Could not mount SPIFFS");
		display.setCursor(0, 0);
		display.println("SPIFFS\nFAIL");
		display.display();
		while (true) ; //TODO: sleep
	}
	
	spi.begin(SD_CLK, SD_MISO, SD_MOSI, SD_CS);
	
	if (!SD.begin(SD_CS, spi, 40000)) {
		Serial.println("Card Mount Failed");
		return;
	}
	uint8_t cardType = SD.cardType();

	if (cardType == CARD_NONE) {
		Serial.println("No SD card attached");
		return;
	}
	Serial.print("SD Card Type: ");
	if (cardType == CARD_MMC) {
		Serial.println("MMC");
	}
	else if (cardType == CARD_SD) {
		Serial.println("SDSC");
	}
	else if (cardType == CARD_SDHC) {
		Serial.println("SDHC");
	}
	else {
		Serial.println("UNKNOWN");
	}

	uint64_t cardSize = SD.cardSize() / (1024 * 1024);
	Serial.printf("SD Card Size: %lluMB\r\n", cardSize);

	listDir(SD, "/", 0);
	
	
	//Connect to access point
	WiFi.disconnect(true);

	delay(1000);
	
	WiFi.onEvent(WiFiStationConnected, SYSTEM_EVENT_STA_CONNECTED);
	WiFi.onEvent(WiFiGotIP, SYSTEM_EVENT_STA_GOT_IP);
	WiFi.onEvent(WiFiStationDisconnected, SYSTEM_EVENT_STA_DISCONNECTED);
	
	WiFi.begin(ssid, psk);
	
	
	
	
	//Create Tasks
	mutex = xSemaphoreCreateMutex();
	Serial.println("[Main] Assigning tasks");
	xTaskCreatePinnedToCore(displayTask, "DisplayTask", 2048 * 2, NULL, 1, NULL, app_cpu);
	xTaskCreatePinnedToCore(measurementTask, "MeasurementTask", 2048, NULL, 1, NULL, app_cpu);
	
	
	//Prime requests
	server.on("/",
		HTTP_GET,
		[](AsyncWebServerRequest* request) {
			messageDisplay = "HTTP Request";
			Serial.println("[Server] index req");
			request->send(SPIFFS, "/index.html", "text/html"); 
			Serial.println("[Server] index req complete");
		});
	server.on("/style.css",
		HTTP_GET,
		[](AsyncWebServerRequest* request) {
			Serial.println("[Server] css req");
			request->send(SPIFFS, "/style.css", "text/css");
			Serial.println("[Server] css req complete");
		});
	server.on("/script.js",
		HTTP_GET,
		[](AsyncWebServerRequest* request) {
			Serial.println("[Server] js req");
			request->send(SPIFFS, "/script.js", "text/javascript");
			Serial.println("[Server] js req complete");
		});
	server.on("/Logo.png",
		HTTP_GET,
		[](AsyncWebServerRequest* request) {
			Serial.println("[Server] png req");
			request->send(SPIFFS, "/Logo.png", "image/png");
			Serial.println("[Server] png req complete");
		}); 
	server.on("/data.csv",
		HTTP_GET,
		[](AsyncWebServerRequest* request) {
			Serial.println("[Server] csv req");
			if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE)
			{
				Serial.println("[Server] csv mutex obtained");
				request->send(SD, "/data.csv", "text/csv"); 
				xSemaphoreGive(mutex);
				Serial.println("[Server] csv sent");
			}
			else
			{
				Serial.println("[Server] csv mutex not obtained");
			}
			Serial.println("[Server] csv req complete");
		}); 
	server.on("/jobmaker.html",
		HTTP_GET,
		[](AsyncWebServerRequest* request) {
			request->send(SPIFFS, "/jobmaker.html", "text/html"); 
		}); 
	server.on("/jobsubmitter.js",
		HTTP_GET,
		[](AsyncWebServerRequest* request) {
			request->send(SPIFFS, "/jobsubmitter.js", "text/javascript");
		});
	server.on("/config.json",
		HTTP_GET,
		[](AsyncWebServerRequest* request) {
			request->send(SPIFFS, "/config.json", "text/json"); 
		}); 
	server.on("/post",
		HTTP_POST, 
		//Callback on POST request
		[](AsyncWebServerRequest *request)
		{
			
		},
		//Callback on POST download begin
		[](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final)
		{
			
		},
		//Callback on download complete
		[](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
		{
			deserializeJson(doc, data);
			
			serializeJsonPretty(doc, jobData);
			
			SPIFFS.remove("/config.json");
			SD.remove("/data.csv");
			
			File file = SPIFFS.open("/config.json", FILE_WRITE);
			File dataFile = SD.open("/data.csv", FILE_WRITE);
			dataFile.close();
	
			if (!file) {
				Serial.println("[Server] Error opening config file for writing");
			}
			else
			{
				if (!file.print(jobData))
				{
					Serial.println("[Server] Could not write config file");
					file.close();
				}
				else
				{
					file.close();
					startJob();
				}
			}
			
			request->send(200, "text/plain", "OK");

		});
	server.onNotFound([](AsyncWebServerRequest* request) {
		request->send(404, "text/plain", "Not Found");
	}); 
	
	//Start server
	server.begin();
	
	startJob();
}

void loop()
{
	delay(1000);
}
