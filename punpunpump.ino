
#define PUMP_COUNT 2
#define FLOAT_SWITCH_PIN 3
#define MODE_SWITCH_PIN 4
#define MANUAL_START_STOP_PIN 5

#define MODE_LED_AUTO_PIN 6
#define MODE_LED_MANUAL_PIN 7

uint8_t pumps_pin[PUMP_COUNT] = {A0, A1};
uint8_t pumps_sleep_led_pin[PUMP_COUNT] = {11, 12};

unsigned long runtime_limit = 3600; //millis
unsigned long sleep_time = 1800;    //millis

unsigned long pumps_start_time[PUMP_COUNT];
unsigned long pumps_stop_time[PUMP_COUNT];
bool pumps_running[PUMP_COUNT];
bool pumps_ready[PUMP_COUNT] = {true, true};

bool IsPumpRunning = false;

enum Mode_t : uint8_t
{
  Auto = 0,
  Manual
};

Mode_t mode;

void setup()
{
  Serial.begin(9600);
  pinMode(MODE_SWITCH_PIN, INPUT_PULLUP);
  pinMode(MANUAL_START_STOP_PIN, INPUT_PULLUP);
  pinMode(MODE_LED_AUTO_PIN, OUTPUT);
  pinMode(MODE_LED_MANUAL_PIN, OUTPUT);
  pinMode(FLOAT_SWITCH_PIN, INPUT_PULLUP);
  for (uint8_t i = 0; i < PUMP_COUNT; i++)
  {
    pinMode(i, OUTPUT);
  }
}

void loop()
{
  if (NeedToFillWater())
  {
    if (!AnyPumpRunning())
    {
      int readyPump = GetReadyPump();
      if (readyPump == -1)
      {
        //Serial.println("No ready pump.");
      }
      else
      {
        StartPump(readyPump);
      }
    }
    else
    {
      PumpRunningCheck();
    }
  }
  else
  {
    for (uint8_t i = 0; i < PUMP_COUNT; i++)
    {
      if (pumps_running[i])
      {
        StopPump(i);
      }
    }
  }
  PumpReadyCheck();
}

void PumpReadyCheck()
{
  IsPumpRunning = false;
  for (uint8_t i = 0; i < PUMP_COUNT; i++)
  {
    if (!pumps_ready[i])
    {
      if (millis() - pumps_stop_time[i] >= sleep_time)
      {
        Serial.print("Pump ");
        Serial.print(i);
        Serial.println(" ready!");
        pumps_ready[i] = true;
      }
    }
    if (pumps_running[i])
    {
      IsPumpRunning = true;
    }
  }
}

void StartPump(uint8_t i)
{
  pumps_start_time[i] = millis();
  digitalWrite(pumps_pin[i], HIGH);
  pumps_running[i] = true;
  Serial.print("Start pump: ");
  Serial.println(i);
}

void StopPump(uint8_t i)
{
  pumps_stop_time[i] = millis();
  digitalWrite(pumps_pin[i], LOW);
  pumps_running[i] = false;
  Serial.print("Stop pump: ");
  Serial.println(i);
}

bool NeedToFillWater()
{
  return digitalRead(FLOAT_SWITCH_PIN) == LOW;
}

void PumpRunningCheck()
{
  for (uint8_t i = 0; i < PUMP_COUNT; i++)
  {
    if (pumps_running[i])
    {
      if (millis() - pumps_start_time[i] >= runtime_limit)
      {
        Serial.print("Pump ");
        Serial.print(i);
        Serial.println(" need to sleep");
        StopPump(i);
        pumps_ready[i] = false;
      }
    }
  }
}

int GetReadyPump()
{
  for (uint8_t i = 0; i < PUMP_COUNT; i++)
  {
    if (pumps_ready[i])
    {
      return i;
    }
  }
  return -1;
}

bool AnyPumpRunning()
{
  return IsPumpRunning;
}