#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <GyverButton.h>
#include <EEManager.h>
#include  <../lib/GyverTimer/GyverTimer.h>

#define ds18Pin 2
#define heaterPin 10
#define compressorPin 12

#define HEAT_DURATION 1800000L // 30 minuts
#define HEAT_PERIOD 259200000L // 3 days

#define ONE_DAY 86400000L
#define ONE_HOUR 3600000L

OneWire ds(ds18Pin);
LiquidCrystal_I2C lcd(0x27, 16, 2);

GTimer updateLcdTimer;
GTimer heaterPeriodTimer;
GTimer heaterDurationTimer;

GTimer timer1;
int getTemp()
{
  static int t = 0;
  byte data[2];
  ds.reset();
  ds.write(0xCC);
  ds.write(0x44);

  if (timer1.isReady())
  {
    ds.reset();
    ds.write(0xCC);
    ds.write(0xBE);
    data[0] = ds.read();
    data[1] = ds.read();
    t = (data[1] << 8) + data[0];
    t = t >> 4;
  }
  return t;
}

void initTimers()
{
  timer1.setInterval(750);
  updateLcdTimer.setInterval(1000);
  heaterPeriodTimer.setTimeout(HEAT_PERIOD);
  heaterDurationTimer.setTimeout(HEAT_DURATION);
  heaterDurationTimer.stop();
}

float timeNormalize(uint32_t time, float base) {
    return floor((time / base) * 100) / 100;
}

void updateRestTimeLcd()
{
  lcd.setCursor(0, 1);
  lcd.print(F("                "));
  lcd.setCursor(0, 1);
  if (heaterPeriodTimer.isEnabled()) {
      lcd.print(F("Holod: "));
      lcd.print(timeNormalize(heaterPeriodTimer.restTime(), ONE_DAY));
      lcd.print(F(" / D"));
  }

  if (heaterDurationTimer.isEnabled()) {
      lcd.print(F("Ottaika: "));
      lcd.print(timeNormalize(heaterDurationTimer.restTime(), ONE_HOUR));
      lcd.print(F(" / H"));
  }
}

void updateLcd(int temp)
{
  lcd.setCursor(0, 0);
  lcd.print(F("T:      *C"));
  lcd.setCursor(3, 0);
  lcd.print(temp);
  updateRestTimeLcd();
}

void setup()
{
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW); // negative pin

  pinMode(7, OUTPUT);
  digitalWrite(7, LOW); // negative pin

  pinMode(heaterPin, OUTPUT);
  pinMode(compressorPin, OUTPUT);

  digitalWrite(heaterPin, LOW);
  digitalWrite(compressorPin, HIGH);

  initTimers();

  lcd.init();
  lcd.backlight();
}

void loop()
{
  if (updateLcdTimer.isReady())
    updateLcd(getTemp());

  if (heaterPeriodTimer.isReady())
  {
    heaterPeriodTimer.stop();
    heaterDurationTimer.start();
    digitalWrite(heaterPin, HIGH);
    digitalWrite(compressorPin, LOW);
  }

  if (!heaterPeriodTimer.isEnabled())
  {
    if (heaterDurationTimer.isReady())
    {
      heaterDurationTimer.stop();
      heaterPeriodTimer.start();
      digitalWrite(heaterPin, LOW);
      digitalWrite(compressorPin, HIGH);
    }
  }
}
