#include <WiFi.h>
#include <ESP32Time.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>
#include <Ticker.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for SSD1306 display connected using software SPI (default case):
#define OLED_MOSI   23
#define OLED_CLK   18
#define OLED_DC    16
#define OLED_CS    5
#define OLED_RESET 17

//Constants
static const BaseType_t pro_cpu = 0;
static const BaseType_t app_cpu = 1;

const char* ssid = "LAPTOP-8JMPRCIB 9360";
const char* psk = "AMDR9270X";

const float scaleVal = 0.000805861f;
const float bias = 0.0f;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
String ip_addr_str;
AsyncWebServer server(80);



void displayTask(void* parameters)
{
	while (true)
	{
		time_t t = time(NULL);
		struct tm *t_st;
		t_st = localtime(&t);
		
		float voltage = analogRead(GPIO_NUM_36) * scaleVal + bias;
		
		display.clearDisplay();
		display.setCursor(0, 0);
		display.setTextSize(1);
		display.printf("%02d-%02d-%04d  %02d:%02d:%02d\n", t_st->tm_mday, 1 + t_st->tm_mon, 1900 + t_st->tm_year, t_st->tm_hour, t_st->tm_min, t_st->tm_sec);
		
		display.setTextSize(2);
		display.print("IPV4:");
		display.println(ip_addr_str);
		
		display.setTextSize(3);
		display.printf("%.4fV\n", voltage);
		display.display();
		
		delay(100);
	}
}

void measurementTask(void* parameters)
{
	while (true)
	{
		delay(10);
	}
}

void logTask(void* parameters)
{
	while (true)
	{
		delay(60000);
	}
}


void setup()
{
	//init LED
	pinMode(LED_BUILTIN, OUTPUT);
	
	//Begin serial Communication
	Serial.begin(115200);
	
	// SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
	if (!display.begin(SSD1306_SWITCHCAPVCC)) {
		Serial.println(F("SSD1306 allocation failed"));
		while (true) ; // TODO: sleep
	}
	
	display.display();
	delay(1000);
	display.clearDisplay();
	display.setTextSize(2); // Draw 2X-scale text
	display.setTextColor(SSD1306_WHITE);
	
	
	//init SPIFFS
	if (!SPIFFS.begin())
	{
		Serial.println("Could not mount SPIFFS");
		display.setCursor(0, 0);
		display.println("SPIFFS\nFAIL");
		display.display();
		while (true) ; //TODO: sleep
	}
	
	//Connect to access point
	Serial.printf("Connecting to %s", ssid);
	display.setCursor(0, 0);
	display.printf("Connecting\n%s", ssid);
	display.display();
	WiFi.begin(ssid, psk);
	while (WiFi.status() != WL_CONNECTED)
	{
		digitalWrite(LED_BUILTIN, HIGH);
		Serial.print(".");
		display.print(".");
		display.display();
		delay(500);
		digitalWrite(LED_BUILTIN, LOW);
		Serial.print(".");
		display.print(".");
		display.display();
		delay(500);
	}
	
	//Print our ip
	ip_addr_str = WiFi.localIP().toString();
	display.clearDisplay();
	Serial.printf("Connected! ip: ");
	Serial.println(ip_addr_str);
	display.setCursor(0, 0);
	display.printf("Connected!\n");
	display.println(ip_addr_str);
	display.display();
	
	//Set Time
	ESP32Time.begin();
	
	//Create Tasks
	xTaskCreatePinnedToCore(displayTask, "DisplayTask", 2048, NULL, 1, NULL, app_cpu);
	xTaskCreatePinnedToCore(logTask, "LoggingTask", 2048, NULL, 1, NULL, app_cpu);
	xTaskCreatePinnedToCore(measurementTask, "MeasurementTask", 2048, NULL, 2, NULL, app_cpu);
	
	//Prime requests
	server.on("/",
		HTTP_GET,
		[](AsyncWebServerRequest* request) {
			request->send(SPIFFS, "/index.html", "text/html"); 
		});
	server.on("/style.css",
		HTTP_GET,
		[](AsyncWebServerRequest* request) {
			request->send(SPIFFS, "/style.css", "text/css");
		});
	server.on("/script.js",
		HTTP_GET,
		[](AsyncWebServerRequest* request) {
			request->send(SPIFFS, "/script.js", "text/javascript");
		});
	server.on("/Logo.png",
		HTTP_GET,
		[](AsyncWebServerRequest* request) {
			request->send(SPIFFS, "/Logo.png", "image/png");
		}); 
	server.on("/data.csv",
		HTTP_GET,
		[](AsyncWebServerRequest* request) {
			request->send(SPIFFS, "/index.html", "text/html"); 
		}); 
	server.onNotFound([](AsyncWebServerRequest* request) {
		request->send(404, "text/plain", "Not Found");
	}); 
	
	//Start server
	server.begin();
}

void loop()
{
	
}
