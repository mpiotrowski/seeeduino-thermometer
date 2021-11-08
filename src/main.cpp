
// // mapping suggestion for AVR, UNO, NANO etc.
// // BUSY -> 7, RST -> 9, DC -> 8, CS-> 10, CLK -> 13, DIN -> 11

#include <DHT.h>
#include <GxEPD.h>
#include <GxGDEH0154D67/GxGDEH0154D67.h>  // 1.54" b/w

#define DHTPIN 2
#define DHTTYPE DHT22

#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

#include <MemoryUsage.h>

GxIO_Class io(SPI, /*CS=*/ SS, /*DC=*/ 8, /*RST=*/ 9); // arbitrary selection of 8, 9 selected for default of GxEPD_Class
GxEPD_Class display(io, /*RST=*/ 9, /*BUSY=*/ 7); // default selection of (9), 7

DHT dht(DHTPIN, DHTTYPE);

#define PAST_READINGS 50

#define PADDING_LEFT 20

short tempF = 0.0;
unsigned short humidity = 0.0;
short heatIdx = 0.0; // 0 - 100

short tempHistory[PAST_READINGS] = {};
unsigned short humdHistory[PAST_READINGS] = {};
int currentReading = 0;

void setup()
{
  Serial.begin(9600);
  display.init(0);
  display.setTextColor(GxEPD_BLACK);
  display.setRotation(1);
  display.eraseDisplay();
  dht.begin();

  memset(&tempHistory, 0, sizeof(short) * PAST_READINGS);
  memset(&humdHistory, 0, sizeof(short) * PAST_READINGS);
}

void readEnvironment()
{
  float h = dht.readHumidity();
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  tempF = f;
  humidity = h;
  heatIdx = dht.computeHeatIndex(f, h, true);

  if(currentReading >= PAST_READINGS)
  {
    currentReading = PAST_READINGS / 2;
    memcpy(tempHistory, tempHistory + PAST_READINGS / 2, PAST_READINGS / 2);
    memcpy(humdHistory, humdHistory + PAST_READINGS / 2, PAST_READINGS / 2);
    memset(tempHistory + PAST_READINGS / 2, 0, sizeof(short) * PAST_READINGS / 2);
    memset(humdHistory + PAST_READINGS / 2, 0, sizeof(short) * PAST_READINGS / 2);
  }
  tempHistory[currentReading] = tempF;
  humdHistory[currentReading] = humidity;
}

inline void drawLine(int b, int l, int mn, int mx, float c)
{
  const float scalingFactor = (95 - 30) / max(mx - mn, 1);

  if(mn == mx)
      display.drawLine(PADDING_LEFT + l, b, PADDING_LEFT + l, b - 0.5 * scalingFactor, GxEPD_BLACK);
    else
      display.drawLine(PADDING_LEFT + l, b, PADDING_LEFT + l, b - (c - mn) * scalingFactor, GxEPD_BLACK);
}

inline float calculateIntermediateValue(short left, short right)
{
      float value = right;
      if(left < right)
      {
        value -= (right - left) / 3.;
      }
      else if(left > right)
      {
        value += (left - right) / 3.;
      }
      return value;
}

inline void displayTemperature()
{
  display.setCursor(0, 0);

  display.setTextSize(2);
  display.print(F("Temp: "));
  display.print(tempF);
  display.print(F(" HI: "));
  display.println(heatIdx);

  short maxTemp = 0;
  short minTemp = 0xFF;
  for(int i = 0; i <= currentReading; i++)
  {
    maxTemp = max(maxTemp, tempHistory[i]);
    minTemp = min(minTemp, tempHistory[i]);
  }

  display.setTextSize(1);
  display.setCursor(0, 30);
  display.println(maxTemp);

  display.setCursor(0, 88);
  display.println(minTemp);

  for(int current = 0; current <= currentReading; current++)
  {
    if(current > 0)
    {
      const short previous = current - 1;
      float value = calculateIntermediateValue(tempHistory[previous], tempHistory[current]);
      drawLine(95, 3 * current - 1, minTemp, maxTemp, value);
    }

    drawLine(95, (3 * current), minTemp, maxTemp, tempHistory[current]);

    if(current < currentReading)
    {
      const short next = current + 1;
      float value = calculateIntermediateValue(tempHistory[next], tempHistory[current]);
      drawLine(95, 3 * current + 1, minTemp, maxTemp, value);
    }
  }
}

inline void displayHumidity()
{
  display.setCursor(0, 100);

  display.setTextSize(2);
  display.print(F("Humidity: "));
  display.println(humidity);

  unsigned short maxHum = 0;
  unsigned short minHum = 0xFF;
  for(int i = 0; i <= currentReading; i++)
  {
    maxHum = max(maxHum, humdHistory[i]);
    minHum = min(minHum, humdHistory[i]);
  }

  display.setTextSize(1);
  display.setCursor(0, 130);
  display.println(maxHum);

  display.setCursor(0, 188);
  display.println(minHum);

  const float scalingFactor = (95 - 30) / max(maxHum - minHum, 1);

  for(int i = 0; i < currentReading; i++)
  {
    if(minHum == maxHum)
      display.drawLine(i + 15, 195, i + 15, 195 - 0.5 * scalingFactor, GxEPD_BLACK);
    else
      display.drawLine(i + 15, 195, i + 15, 195 - (humdHistory[i] - minHum) * scalingFactor, GxEPD_BLACK);
  }
}

void displayEnvironment()
{
  displayTemperature();
  displayHumidity();
}

void loop()
{
  readEnvironment();

  display.drawPaged(displayEnvironment);

  delay(10000);

  currentReading++;
}
