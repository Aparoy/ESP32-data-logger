#include <SPIFFS.h>


void setup() {
	Serial.begin(115200);
	
	if (!SPIFFS.begin(false))
	{
		Serial.println("SPIFFS Mount Failed");
		return;
	}
 
	File rootDir = SPIFFS.open("/");
	if (rootDir)
	{
		for (File file = rootDir.openNextFile(); file; file = rootDir.openNextFile())
		{
			Serial.printf("[%s (%d bytes)]\r\n", file.name(), file.size());
			while (file.available())
			{
				String str = file.readStringUntil('\n');
				Serial.printf("\t%s\r\n", str.c_str());
			}
		}
	}
}

void loop() {
	
}