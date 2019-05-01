
#define PUMP_COUNT 2
#define FLOAT_SWITCH_PIN 4
#define MODE_SWITCH_PIN 2
#define MANUAL_START_STOP_PIN 3

#define MODE_LED_AUTO_PIN 6
#define MODE_LED_MANUAL_PIN 7

#define READY_LED_PIN 13
#define TANK_FULL_LED_PIN 10

uint8_t pumps_pin[PUMP_COUNT] = {A0, A1};
uint8_t pumps_sleep_led_pin[PUMP_COUNT] = {11, 12};

unsigned long runtime_limit = 3000; //millis
unsigned long sleep_time = 10000;   //millis

unsigned long pumps_start_time[PUMP_COUNT];
unsigned long pumps_stop_time[PUMP_COUNT];
bool pumps_running[PUMP_COUNT];
bool pumps_ready[PUMP_COUNT] = {true, true};

bool IsPumpRunning = false;
volatile byte ModeButtonState = HIGH;
volatile byte StartStopButtonState = HIGH;
byte lastModeButtonState = HIGH;
byte lastStartStopButtonState = HIGH;

enum Mode_t : uint8_t
{
  Auto = 0,
  Manual
};

Mode_t mode;

void Init()
{
  pinMode(MODE_SWITCH_PIN, INPUT_PULLUP);
  pinMode(MANUAL_START_STOP_PIN, INPUT_PULLUP);
  pinMode(READY_LED_PIN, OUTPUT);
  pinMode(MODE_LED_AUTO_PIN, OUTPUT);
  pinMode(MODE_LED_MANUAL_PIN, OUTPUT);
  pinMode(FLOAT_SWITCH_PIN, INPUT_PULLUP);
  pinMode(TANK_FULL_LED_PIN, OUTPUT);
  for (uint8_t i = 0; i < PUMP_COUNT; i++)
  {
    pinMode(pumps_pin[i], OUTPUT);
    pinMode(pumps_sleep_led_pin[i], OUTPUT);
  }
  attachInterrupt(digitalPinToInterrupt(MODE_SWITCH_PIN), OnModeButtonPress, FALLING);
  attachInterrupt(digitalPinToInterrupt(MANUAL_START_STOP_PIN), OnStartStopPress, FALLING);
}

void setup()
{
  Init();
  Serial.begin(9600);
  ModeChangeCheck();
  digitalWrite(READY_LED_PIN, HIGH);
  Serial.println("Ready");
}

void loop()
{
  if (mode == Auto)
  {
    if (NeedToFillWater())
    {
      digitalWrite(TANK_FULL_LED_PIN, LOW);
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
      digitalWrite(TANK_FULL_LED_PIN, HIGH);
      StopRunningPump();
    }
  }
  else
  {
    //Manual
  }
  PumpReadyCheck();
  ButtonCheck();
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
        digitalWrite(pumps_sleep_led_pin[i], LOW);
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
        Serial.println(" working hard, need to sleep");
        digitalWrite(pumps_sleep_led_pin[i], HIGH);
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

void StopRunningPump()
{
  for (uint8_t i = 0; i < PUMP_COUNT; i++)
  {
    if (pumps_running[i])
    {
      StopPump(i);
    }
  }
}

void OnModeButtonPress()
{
  ModeButtonState = !ModeButtonState;
}

void OnStartStopPress()
{
  StartStopButtonState = !StartStopButtonState;
}

void ModeChangeCheck()
{
  if (mode == Auto)
  {
    digitalWrite(MODE_LED_AUTO_PIN, HIGH);
    digitalWrite(MODE_LED_MANUAL_PIN, LOW);
  }
  else
  {
    digitalWrite(MODE_LED_AUTO_PIN, LOW);
    digitalWrite(MODE_LED_MANUAL_PIN, HIGH);
    StopRunningPump();
  }
}

void ToggleMode()
{
  Serial.print("Mode changed to ");
  if (mode == Auto)
  {
    mode = Manual;
    Serial.println("Manual");
  }
  else
  {
    mode = Auto;
    Serial.println("Auto");
  }
  ModeChangeCheck();
}

void TogglePump()
{
  if (mode == Manual)
  {
    digitalWrite(pumps_pin[0], !digitalRead(pumps_pin[0]));
  }
}

void ButtonCheck()
{
  if (lastModeButtonState != ModeButtonState)
  {
    lastModeButtonState = ModeButtonState;
    ToggleMode();
  }
  else if (lastStartStopButtonState != StartStopButtonState)
  {
    lastStartStopButtonState = StartStopButtonState;
    TogglePump();
    Serial.print("Start/Stop changed!");
    Serial.println(StartStopButtonState);
  }
}