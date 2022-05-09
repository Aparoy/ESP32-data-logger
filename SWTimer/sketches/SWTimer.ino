static TimerHandle_t one_shot_timer = NULL;
static TimerHandle_t auto_reload_timer = NULL;

void myTimerCallback(TimerHandle_t xTimer)
{
	if ((uint32_t)pvTimerGetTimerID(xTimer) == 0)
	{
		Serial.println("One-Shot Timer Expired");
	}
	
	if ((uint32_t)pvTimerGetTimerID(xTimer) == 1)
	{
		Serial.println("Auto-Reload Timer Expired");
	}
}


void setup()
{
	Serial.begin(115200);
	//Create one-shot timer
	one_shot_timer = xTimerCreate("One-shot timer", 2000 / portTICK_PERIOD_MS, pdFALSE, (void*)0, myTimerCallback);
	if (one_shot_timer == NULL)
	{
		Serial.println("Could not create timer");
	}else
	{
		xTimerStart(one_shot_timer, portMAX_DELAY);
	}
	//Create auto-reload timer
	auto_reload_timer = xTimerCreate("Auto-reload timer", 1000 / portTICK_PERIOD_MS, pdTRUE, (void*)1, myTimerCallback);
	if (one_shot_timer == NULL || auto_reload_timer == NULL)
	{
		Serial.println("Could not create timer");
	}
	else
	{
		xTimerStart(auto_reload_timer, portMAX_DELAY);
	}
}

void loop()
{
	
}
