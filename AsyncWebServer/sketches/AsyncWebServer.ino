#include <WiFi.h>
#include <SPIFFS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);

const char* ssid = "DRTARUN 0186";
const char* password = "U66[27q3";

const char* PARAM_MESSAGE = "message";

void setup() {

	Serial.begin(115200);
	
	//init SPIFFS
	if (!SPIFFS.begin())
	{
		Serial.println("Could not mount SPIFFS");
		while (true) ; //TODO: sleep
	}
	
	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);
	if (WiFi.waitForConnectResult() != WL_CONNECTED) {
		Serial.printf("WiFi Failed!\n");
		return;
	}

	Serial.print("IP Address: ");
	Serial.println(WiFi.localIP());

	server.on("/",
		HTTP_GET,
		[](AsyncWebServerRequest *request) {
			request->send(SPIFFS, "/index.html", "text/html"); 
		});
	
	server.on("/style.css",
		HTTP_GET,
		[](AsyncWebServerRequest *request) {
			request->send(SPIFFS, "/style.css", "text/css"); 
		});
	

	server.onNotFound([](AsyncWebServerRequest *reqhandler){
		//function body
		reqhandler->send(404, "text/plain", "not found");
	});

	server.begin();
}

void loop() {
}