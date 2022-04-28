#include <WebSocketsServer.h>


enum Message : uint8_t
{
	toggleLED,
	getLEDState
};



//constants
const char* ssid = "LAPTOP-8JMPRCIB 9360";
const char* psk = "AMDR9270X";

bool ledState = false;

//globals
WebSocketsServer webSocket = WebSocketsServer(80);

void handleEvent(uint8_t num, uint8_t* msg)
{
	switch (*msg)
	{
	case Message::toggleLED:
		ledState = !ledState;
		digitalWrite(LED_BUILTIN, ledState);
		break;
	case Message::getLEDState:
		if (ledState)
			webSocket.sendTXT(num, "LED is on\r\n");
		else
			webSocket.sendTXT(num, "LED is off\r\n");
		break;
	default:
		Serial.printf("[%u] Command not recognized!\r\n", num);
		break;
	}
	
}

//called when receiving message
void onWebsocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length)
{
	switch (type)
	{
	case WStype_DISCONNECTED:
		Serial.printf("[%u] has disconnected!\r\n", num);
		break;
	case WStype_CONNECTED:
		Serial.printf("[%u] has connected from ip: %s\r\n", num, webSocket.remoteIP(num).toString().c_str());
		break;
	case WStype_BIN:
		handleEvent(num, payload);
		break;
		
	default:
		break;
	}
}


void setup()
{
	//init the pin
	pinMode(LED_BUILTIN, OUTPUT);
	
	//Start serial port
	Serial.begin(115200);
	
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
	
	//Start websocket server and prime with a task
	webSocket.begin();
	webSocket.onEvent(onWebsocketEvent);
	
}

void loop()
{
	webSocket.loop();
}
