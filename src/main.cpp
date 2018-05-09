#include "Config.h"

#include <Arduino.h>
#include <avr/pgmspace.h>
#include <Wire.h>
#include <SPI.h>
#include <RtcDS3231.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <RBD_Timer.h>
#include <BH1750.h>
#include <BME280I2C.h>
#include <LiquidCrystal_PCF8574.h>

#include <MySensors.h>

OneWire oneWire(DS18B20_PIN);

RtcDS3231<TwoWire> Rtc(Wire);
RtcDateTime now;

RBD::Timer timer;
RBD::Timer lcdBacklight;

MyMessage msgAirPump(MY_ID_AIRPUMP, V_STATUS);
bool AirPumpOff   = true;
MyMessage msgWaterPump(MY_ID_WATERPUMP, V_STATUS);
bool WaterPumpOff = true;
MyMessage msgGrowlight(MY_ID_GROWLIGHT, V_LIGHT);
bool GrowLightOff = true;
MyMessage msgHeater(MY_ID_HEATER, V_STATUS);
bool HeaterOff    = true;

BME280I2C bmeAmbient;
bool bmePresent = false;
MyMessage msgBMEPressure(MY_ID_BME_BARO, V_PRESSURE);
MyMessage msgBMETemp(MY_ID_BME_TEMP,V_TEMP);
MyMessage msgBMEHumi(MY_ID_BME_HUMI,V_HUM);

DallasTemperature ds18b20_sensors(&oneWire);
MyMessage msgDS18Temp_1(MY_ID_DS18_TEMP_1,V_TEMP);

BH1750 luxMeter(ADDR_LUX);
MyMessage msgLuxmeter(MY_ID_LUX, V_LEVEL);

LiquidCrystal_PCF8574 lcd(ADDR_LCD);


#ifdef DEBUGGERY
void printDateTime(const RtcDateTime& dt);
#endif

void doUpdateSensors(void);

void setup(){
// set pump control relay pins to HIGH to prevent pumps running unexpectedly
  pinMode(AIRPUMP_PIN, OUTPUT);digitalWrite(AIRPUMP_PIN, HIGH);
  pinMode(WATERPUMP_PIN, OUTPUT);digitalWrite(WATERPUMP_PIN, HIGH);

  Serial.begin(115200);
  Serial.println("Starting up...");

  Rtc.Begin();
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);debugLog("Compiled epoch: ", compiled.Epoch32Time());
  if (!Rtc.IsDateTimeValid())
    {
      Serial.println(F("RTC lost confidence in the DateTime at setup!"));
      Rtc.SetDateTime(compiled);
      requestTime();
    };
  if (!Rtc.GetIsRunning())
    {
      Serial.println(F("RTC was not actively running, starting now"));
      Rtc.SetIsRunning(true);
    };
  now = Rtc.GetDateTime();
  if (now < compiled)
    {
      Serial.println(F("RTC is older than compile time!  (Updating DateTime)"));
      Rtc.SetDateTime(compiled);
    }
  else if (now > compiled)
    {
      Serial.println(F("RTC is newer than compile time. (this is expected)"));
    }
  else if (now == compiled)
    {
      Serial.println(F("RTC is the same as compile time! (not expected but all is fine)"));
    };

  Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeAlarmBoth);

  debugLog("Epoch now:", now.Epoch32Time());
  ds18b20_sensors.begin();
  uint8_t i=0;
  while(i<10 && !bmePresent)
    {
      bmePresent=bmeAmbient.begin();
      i++;
    };
  luxMeter.begin(BH1750_CONTINUOUS_HIGH_RES_MODE);
  timer.setTimeout(MS_SEND_INTERVAL);
  lcdBacklight.setTimeout(MS_LCD_BACKLIGHT);
  lcdBacklight.stop();

  lcd.begin(20, 4);lcd.clear();lcd.setBacklight(60);
  doUpdateSensors();
}


void loop() {
    if (!Rtc.IsDateTimeValid())
      {
        Serial.println(F("RTC lost confidence in the DateTime! Again..."));
        debugLog("It thinks it's ", Rtc.GetDateTime().Epoch32Time());
        requestTime();
      };

    now = Rtc.GetDateTime();
    uint8_t nHour = now.Hour();
    uint8_t nMin  = now.Minute();
    if((nMin>=motorControl[nHour][0]) && (nMin<motorControl[nHour][1]))
      {
        if (AirPumpOff)
          {
            digitalWrite(AIRPUMP_PIN, LOW); AirPumpOff = false;
            send(msgAirPump.set(1));
            debugLog("Air pump turned on at: ", now.Epoch32Time());
          }
      } else {
        if(!AirPumpOff)
          {
            digitalWrite(AIRPUMP_PIN, HIGH);AirPumpOff = true;
            send(msgAirPump.set(0));
            debugLog("Air pump turned off at: ", now.Epoch32Time());
          }
      };
    if((nMin>=motorControl[nHour][2]) && (nMin<motorControl[nHour][3]))
      {
        if(WaterPumpOff)
          {
            digitalWrite(WATERPUMP_PIN, LOW); WaterPumpOff = false;
            send(msgWaterPump.set(1));
            debugLog("Water pump turned on at: ", now.Epoch32Time());
          }
      } else {
        if(!WaterPumpOff)
          {
            digitalWrite(WATERPUMP_PIN, HIGH); WaterPumpOff = true;
            send(msgWaterPump.set(0));
            debugLog("Water pump turned off at: ", now.Epoch32Time());
          }
      };

    if(timer.onRestart())doUpdateSensors();
    if(lcdBacklight.isExpired())lcd.setBacklight(0);
    wait(5000);
}

void presentation()
{
	// Send the sketch version information to the gateway and Controller
	sendSketchInfo("HydroPoo", "2.3");
	// Register all sensors to gw (they will be created as child devices)
	//present(MY_ID_LCD, S_INFO, "LCD 2004");
  present(MY_ID_DS18_TEMP_1, S_TEMP, "DWC-1 temp"); debugLog("DS18B20: ", true);
  present(MY_ID_BME_TEMP, S_TEMP, "Ambient temp");debugLog("Ambient temp: ", true);
  present(MY_ID_BME_HUMI, S_HUM, "Humidity");debugLog("Ambient humidity: ", true);
  present(MY_ID_BME_BARO, S_BARO, "Barometer");debugLog("Ambient pressure: ", true);
  present(MY_ID_AIRPUMP, S_BINARY, "Air pump"); debugLog("Air Pump: ", AirPumpOff);
  present(MY_ID_WATERPUMP, S_BINARY, "Water pump"); debugLog("Water pump: ", WaterPumpOff);
  present(MY_ID_GROWLIGHT, S_LIGHT, "Grow light"); debugLog("Grow Light: ", GrowLightOff);
  present(MY_ID_HEATER, S_BINARY, "Heater pad"); debugLog("Heater", HeaterOff);
  present(MY_ID_LUX, S_LIGHT_LEVEL, "Luxmeter"); debugLog("Lux meter: ", true);
  debugLog("Presented everything!", " Yes, really!");
}

void receiveTime (unsigned long ts)
{
  debugLog("Got timestamp: ", ts);
  now.InitWithEpoch32Time(ts);
  Rtc.SetDateTime(now);
}

void doUpdateSensors(void)
{
  float temp(NAN), hum(NAN), pres(NAN);
  ds18b20_sensors.requestTemperatures();
  if(bmePresent)
  {
    bmeAmbient.read(pres, temp, hum, PU_HPA, true);
    send(msgBMEHumi.set(hum,2));debugLog("Ambient humidity: ", hum);
    send(msgBMETemp.set(temp,2));debugLog("Ambient temp: ", temp);
    send(msgBMEPressure.set(pres, 2));debugLog("Ambient pressure: ", hPa2mmHg(pres));
  };
  float t=ds18b20_sensors.getTempCByIndex(0);
  debugLog3("DS18B20 Temp: ", t,2);
  send(msgDS18Temp_1.set(t,2));
  uint16_t lux = luxMeter.readLightLevel();
  debugLog("Lux: ", lux);
  debugLog("Hour: ", now.Hour());
  debugLog("Minute: ", now.Minute());
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("00:00 ###.# C Hr ##%");
  lcd.setCursor(0, 1); lcd.print("##### mmHg  ##### Lx");
  lcd.setCursor(0, 2); lcd.print("AP: # WP: # @ ##.# C");
  lcd.setCursor(0, 3); lcd.print(" Light ### Heat ### ");
  lcd.setCursor(0, 0); lcd.print((now.Hour()<9)?"0":""); lcd.print(now.Hour());
  lcd.setCursor(3, 0); lcd.print((now.Minute()<9)?"0":""); lcd.print(now.Minute());
  lcd.setCursor(6, 0); lcd.print((temp<0)?"-":" "); lcd.print(((temp>(-10))&&(temp<10))?"0":""); lcd.print(temp,1);
  lcd.setCursor(17, 0); lcd.print((hum<10)?"0":""); lcd.print((int)hum);
  lcd.setCursor(0,1); lcd.print((pres<1000)?"0":""); lcd.print(hPa2mmHg((int)pres));
  lcd.setCursor(12, 1); lcd.print((lux<10000)?"0":""); lcd.print((lux<1000)?"0":"");
  lcd.print((lux<100)?"0":""); lcd.print((lux<10)?"0":""); lcd.print(lux);
  lcd.setCursor(4, 2); lcd.print(AirPumpOff?"O":"R");
  lcd.setCursor(10, 2); lcd.print(WaterPumpOff?"O":"R");
  lcd.setCursor(14, 2); lcd.print(t,1);
  lcd.setCursor(7, 3); lcd.print(GrowLightOff?"OFF":" ON");
  lcd.setCursor(16, 3); lcd.print(HeaterOff?"OFF":" ON");
  send(msgLuxmeter.set(lux));
  send(msgAirPump.set(AirPumpOff?0:1));
  send(msgWaterPump.set(WaterPumpOff?0:1));
  send(msgHeater.set(HeaterOff?0:1));
  send(msgGrowlight.set(GrowLightOff?0:1));
  debugLog("Sensors sensored!", "");
  lcd.setBacklight(60); lcdBacklight.restart();
}

void receive(const MyMessage &message)
{
#ifdef DEBUGGERY
  Serial.print(F("Message: [["));
  Serial.print(message.sensor);Serial.print(".");
  Serial.print(message.type); Serial.print(":");
  Serial.print(message.getBool());
  Serial.println("]]");
#endif
  if(message.type==V_STATUS)
  {
    switch(message.sensor)
    {
      case MY_ID_AIRPUMP:   if(message.getBool()!=AirPumpOff) {
                              AirPumpOff = message.getBool();
                              digitalWrite(AIRPUMP_PIN, (AirPumpOff?HIGH:LOW));
                              send(msgAirPump.set(AirPumpOff));
                              debugLog("Air pump toggled: ", (AirPumpOff?"Off":"On"));
                              lcd.setCursor(4, 2); lcd.print(AirPumpOff?"O":"R");
                            };
                            break;
      case MY_ID_WATERPUMP: if(message.getBool()!=WaterPumpOff) {
                              WaterPumpOff = message.getBool();
                              digitalWrite(WATERPUMP_PIN, (WaterPumpOff?HIGH:LOW));
                              send(msgWaterPump.set(WaterPumpOff));
                              debugLog("Water pump toggled: ", (WaterPumpOff?"Off":"On"));
                              lcd.setCursor(10, 2); lcd.print(WaterPumpOff?"O":"R");
                            };
                            break;
      case MY_ID_GROWLIGHT: if(message.getBool()!=GrowLightOff) {
                            GrowLightOff = message.getBool();
                            digitalWrite(LIGHT_PIN, (GrowLightOff?HIGH:LOW));
                            send(msgGrowlight.set(GrowLightOff));
                            debugLog("Grow light toggled: ", (GrowLightOff?"Off":"On"));
                            lcd.setCursor(7, 3); lcd.print(GrowLightOff? "OFF":" ON");
                          };
                          break;
      case MY_ID_HEATER:  if(message.getBool()!=HeaterOff) {
                            HeaterOff = message.getBool();
                            digitalWrite(HEATER_PIN, (HeaterOff?HIGH:LOW));
                            send(msgHeater.set(HeaterOff));
                            debugLog("Heater pad toggled: ", (HeaterOff?  "Off":"On"));
                            lcd.setCursor(16, 3); lcd.print(HeaterOff? "OFF":" ON");
                          };
                          break;
    };
  }
}

#ifdef DEBUGGERY
#define countof(a) (sizeof(a) / sizeof(a[0]))
void printDateTime(const RtcDateTime& dt)
{
    char datestring[20];
    snprintf_P(datestring,
            countof(datestring),
            PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
            dt.Month(),
            dt.Day(),
            dt.Year(),
            dt.Hour(),
            dt.Minute(),
            dt.Second() );
    Serial.println(datestring);
}
#endif
