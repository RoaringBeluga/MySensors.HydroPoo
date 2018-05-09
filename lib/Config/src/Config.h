#ifndef DSU_CONFIG_H
#define DSU_CONFIG_H
/*
MySensors settings
*/
//#define MY_DEBUG
#define MY_RADIO_NRF24

#include <Arduino.h>
#include <MySensors.h>
/*
Debug output settings
*/
#define DEBUGGERY // Debug output is ON if defined

#ifdef DEBUGGERY
#define debugLog(msg, value)  {Serial.print(F(msg));Serial.println(value);}
#define debugLog3(msg, value, precision)  {Serial.print(F(msg));Serial.println(value, precision);}
#else
#define debugLog(msg, value)
#definr debugLog3(msg, value, precision)
#endif

/*
Pin definitions
*/
#define ALARM_PIN   2 // DS3201 alarm interupt
#define DHT_PIN     3 // DHT22 sensor connection
#define DS18B20_PIN 4 // OneWire DS18B20 sensor(s)

#define AIRPUMP_PIN   5 // Air pump relay control pin, relay 1 @4-relay board
#define WATERPUMP_PIN 6 // Water pump relay control pin, relay 2 @4-relay board
#define LIGHT_PIN     7 // Lighting relay control pin, relay 3
#define HEATER_PIN    8 // Heater pad relay control pin, relay 4

/*
Should not be needed as we're almost out of GPIOs...
The values are just some placeholders!

#define SDA_PIN 9 // Default is ADC4
#define SCL_PIN 10 // Default is ADC5
*/

/*
Some provisions for SD card logger...
Although for now the SPI interface is taken by nRF24 radio and anyway
we're almost out of pins as it is. Also, builing this as MySensors node
shoul eliminate the need for the SD logger.

#define SD_MOSI 11
#define SD_MISO 12
#define SD_SCK  13
#define SD_CS   10
*/

/*
Pump runtimes... is it even needed?..
*/
#define AIR_RUN           1200000 // 20 minutes in milliseconds
#define WATER_RUN         60000   // 1 minute in milliseconds
/*
Sensor reporting interval
*/
#define MS_SEND_INTERVAL  300000   // Sensors update interval
#define MS_LCD_BACKLIGHT  10000    // Backlight interval

/*
Pressure units for BME280
  B000 = Pa
  B001 = hPa
  B010 = Hg
  B011 = atm
  B100 = bar
  B101 = torr
  B110 = N/m^2
  B111 = psi
*/
#define PU_PA     B000
#define PU_HPA    B001
#define PU_IN_HG  B010
#define PU_ATM    B011
#define PU_BAR    B100
#define PU_TORR   B101
#define PU_NMsq   B110
#define PU_PSI    B111
// ... and turning inches into millimeters...
#define in2mmHg(x)  (x*25.4)
// ... and the same with hPa...
#define hPa2mmHg(x) (x/1.3332239)

/*
IIC addresses
*/
#define ADDR_LCD    0x3F
#define ADDR_RTC    0x68
#define ADDR_EEPROM 0x57
#define ADDR_LUX    0x23
#define ADDR_BME280 0x76

// ON/OFF times for air and water pumps
const uint8_t motorControl[24][4]  = {
  {00, 15, 00, 05}, {00, 15, 00, 05}, {00, 15, 00, 05}, // 00:XX, 01:XX, 02:XX
  {00, 15, 00, 05}, {00, 15, 00, 05}, {00, 15, 00, 05}, // 03:XX, 04:XX, 05:XX
  {00, 15, 00, 05}, {00, 15, 00, 05}, {00, 15, 00, 05}, // 06:XX, 07:XX, 08:XX
  {00, 30, 00, 05}, {00, 30, 00, 05}, {00, 30, 00, 05}, // 09:XX, 10:XX, 11:XX
  {00, 30, 00, 05}, {00, 30, 00, 05}, {00, 30, 00, 05}, // 12:XX, 13:XX, 14:XX
  {00, 30, 00, 05}, {00, 30, 00, 05}, {00, 30, 00, 05}, // 15:XX, 16:XX, 17:XX
  {00, 30, 00, 05}, {00, 30, 00, 05}, {00, 30, 00, 05}, // 18:XX, 19:XX, 20:XX
  {00, 15, 00, 05}, {00, 15, 00, 05}, {00, 15, 00, 05}  // 21:XX, 22:XX, 23:XX
};

// Various info for MySensors
#define MY_ID_LCD         100 // 1:LCD
#define MY_ID_BME_TEMP    10  // 2:BME280 Temperature
#define MY_ID_BME_HUMI    11  // 3:BME280 Humidity
#define MY_ID_BME_BARO    12  // 4:BME280 barometer
#define MY_ID_DS18_TEMP_1 20  // 5:DS18B20 Temperature #1
#define MY_ID_LUX         30  // 6:BH1750 luxmeter
#define MY_ID_AIRPUMP     00  // 7:Airpump relay
#define MY_ID_WATERPUMP   01  // 8:Waterpump relay
#define MY_ID_GROWLIGHT   02  // 9:Grow light
#define MY_ID_HEATER      03  // 10:Heater relay

#endif
