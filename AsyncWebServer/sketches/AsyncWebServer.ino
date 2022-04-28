#include <WiFi.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>

//constants
const char* ssid = "LAPTOP-8JMPRCIB 9360";
const char* psk = "AMDR9270X";


//globals
AsyncWebServer server(80);

//called when request webpage
void onIndexRequest(AsyncWebServerRequest* request)
{
	request->send(SPIFFS, "/index.html", "text/html");
}

//called when request stylesheet
void onCSSRequest(AsyncWebServerRequest* request)
{
	request->send(SPIFFS, "/style.css", "text/css");
}

void onJSRequest(AsyncWebServerRequest* request)
{
	request->send(SPIFFS, "/script.js", "text/javascript");
}

void onPNGRequest(AsyncWebServerRequest* request)
{
	request->send(SPIFFS, "/Logo.png", "image/png");
}

void onDataRequest(AsyncWebServerRequest* request)
{
	request->send(SPIFFS, "/data.csv", "text/csv");
}

//404
void onPageNotFound(AsyncWebServerRequest* request)
{
	request->send(404, "text/plain", "Not Found");
}

void setup()
{
	
	//Start serial port
	Serial.begin(115200);
	
	//init SPIFFS
	if (!SPIFFS.begin())
	{
		Serial.println("Could not mount SPIFFS");
		while (true) ;
	}
	
	//Connect to access point
	Serial.printf("Connecting to %s", ssid);
	WiFi.begin(ssid, psk);
	while (WiFi.status() != WL_CONNECTED)
	{
		delay(500);
		Serial.print(".");
	}
	
	//Print our ip
	Serial.printf("Connected! ip: %s\r\n", WiFi.localIP().toString().c_str());
	
	//Prime requests
	server.on("/", HTTP_GET, onIndexRequest);
	server.on("/style.css", HTTP_GET, onCSSRequest);
	server.on("/script.js", HTTP_GET, onJSRequest);
	server.on("/Logo.png", HTTP_GET, onPNGRequest);
	server.on("/data.csv", HTTP_GET, onDataRequest);
	server.onNotFound(onPageNotFound);
	
	//Start server
	server.begin();

	
}

void loop()
{

}
