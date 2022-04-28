#include <WiFi.h>
#include <ESP32Time.h>

const char* ssid = "LAPTOP-8JMPRCIB 9360";
const char* psk = "AMDR9270X";

void setup() {
	
	Serial.begin(115200);

	Serial.printf("connecting to %s ...\r\n", ssid);
	WiFi.begin(ssid, psk);
	if (WiFi.waitForConnectResult() != WL_CONNECTED) {
		Serial.println("WiFi connection failed:(");
		while (1) delay(1000);
	}

	ESP32Time.begin(); //< adjusted the time
}

void loop() {
	time_t t = time(NULL);
	struct tm *t_st;
	t_st = localtime(&t);
	Serial.printf("%d-%d-%d,%d:%d\r\n", t_st->tm_mday, 1 + t_st->tm_mon, 1900 + t_st->tm_year, t_st->tm_hour, t_st->tm_min);
	
	
	delay(1000);
}