/*
  Monitor cabin power
    C. Catlett Apr 2019
    2020-Oct
      - added MQTT to connect to remote Home Assistant
      - added ds18b20 temperature sensor to monitor crawlspace
                                          
 */

#include <Particle.h>
#include <MQTT.h>
#include "secrets.h"
#include <OneWire.h>
#include <DS18B20.h>

FuelGauge fuel;
#define REPORT          3600007  // Battery lasts 12-18h so update every ~1h (1)
#define TEN_MIN          599999  // watch power at ~10 min (600k ms) intervals
#define FIVE_MIN         314159  // watch more closely; every ~5min (300k ms)
#define LINE_PWR        1        // we're plugged in (5 means we are on battery)
float fuelPercent     = 0;
bool  TimeToCheck     = TRUE;
bool  TimeToReport    = TRUE;
bool  PowerIsOn       = TRUE;
int   powerSource     = 0;

// DS18B20 temperature sensor (needs libraries OneWire and DS18B20)
DS18B20  sensor(D1, true);
#define MAXRETRY    4             // max times to poll temperature pin before giving up
double crawlTemp;
// danger tripwire settings
float danger        = 35.00;    // this is the threshold we define as "pipe freeze eminent"
float allGood       = 37.00;    // not gonna relax until we are seeing temperature go back solid up above danger
float Freezing      = 32.00;    // now we are really in trouble
bool  inDanger      = FALSE;    // start with a clean slate!

/*
 * MQTT parameters
 */
#define MQTT_KEEPALIVE 30 * 60              // 30 minutes but afaict it's ignored...
/*
 * When you configure Mosquitto Broker MQTT in HA you will set a
 * username and password for MQTT - plug these in here if you are not
 * using a secrets.h file. (and comment out the #include "sectets.h" line above)
 */
//const char *HA_USR = "your_ha_mqtt_usrname";
//const char *HA_PWD = "your_ha_mqtt_passwd";
//uncomment this line and fill in w.x.y.z if you are going by IP address,:
//  byte MY_SERVER[] = { 73, 246, 85, 17 };
// OR this one if you are doing hostname (filling in yours)
//  #define MY_SERVER "your.mqtt.broker.tld"
const char *CLIENT_NAME = "photon";
// Topics - these are what you watch for as triggers in HA automations
const char *TOPIC_A = "ha/cabin/powerLevel";
const char *TOPIC_B = "ha/cabin/powerOK";
const char *TOPIC_C = "ha/cabin/powerOUT";
const char *TOPIC_D = "ha/cabin/crawlTemp";
const char *TOPIC_E = "ha/cabin/crawlOK";
const char *TOPIC_F = "ha/cabin/crawlWarn";
const char *TOPIC_G = "ha/cabin/crawlFreeze";

// MQTT functions
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void timer_callback_send_mqqt_data();    
 // MQTT callbacks implementation (not used here but required)
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
     char p[length + 1];
     memcpy(p, payload, length);
     p[length] = 0; 
     Particle.publish("mqtt", p, 3600, PRIVATE);
 }

MQTT client(MY_SERVER, 1883, MQTT_KEEPALIVE, mqtt_callback);

bool DEBUG = TRUE;

Timer powerTimer(FIVE_MIN, checkPower);
Timer reportTimer(REPORT, reportPower);

void setup() {
    Time.zone (-5);
    Particle.syncTime();
    fuelPercent = fuel.getSoC();
    client.connect(CLIENT_NAME, HA_USR, HA_PWD);
    if (DEBUG) Particle.publish("DEBUG", "DEBUG=TRUE", PRIVATE);
    // check MQTT 
    if (client.isConnected()) {
        Particle.publish("mqtt_startup", "Connected to HA", 3600, PRIVATE);
      } else {
        Particle.publish("mqtt_startup", "Failed to connect to HA - check IP address, username, passwd", 3600, PRIVATE);
    }
    powerTimer.start();
    reportTimer.start();
}

void loop() {

    if (TimeToCheck) {
        TimeToCheck = FALSE;
        fuelPercent = fuel.getSoC(); 
        powerSource = System.powerSource();
        if (powerSource == LINE_PWR) {
            PowerIsOn = TRUE;
            Particle.publish("POWER-start ON", String(powerSource), PRIVATE);
            tellHASS(TOPIC_B, String(fuelPercent));
            powerTimer.changePeriod(TEN_MIN);
        } else {
            PowerIsOn = FALSE;
            Particle.publish("POWER-start OFF", String(powerSource), PRIVATE);
            tellHASS(TOPIC_C, String(powerSource));
            powerTimer.changePeriod(FIVE_MIN);
        }
        // check crawlspace
        crawlTemp = getTemp();
        if (crawlTemp > danger) tellHASS(TOPIC_E, String(crawlTemp));
    }

    if (TimeToReport) {
      TimeToReport = FALSE;
      tellHASS(TOPIC_A, String(fuelPercent));
      if (PowerIsOn) {  tellHASS(TOPIC_B, String(fuelPercent));
              } else {  tellHASS(TOPIC_C, String(fuelPercent)); }
      tellHASS(TOPIC_D, String(crawlTemp));
      if (crawlTemp > allGood)   tellHASS(TOPIC_E, String(crawlTemp));
      if (crawlTemp < danger)    tellHASS(TOPIC_F, String(crawlTemp));
      if (crawlTemp < Freezing)  tellHASS(TOPIC_G, String(crawlTemp));
    }
} 
/************************************/
/***         FUNCTIONS       ***/
/************************************/

// Checking timer interrupt handler
void checkPower() {  TimeToCheck = TRUE;  }

// Reporting timer interrupt handler
void reportPower() {  TimeToReport = TRUE;  }

// Report to HASS via MQTT
void tellHASS (const char *ha_topic, String ha_payload) {    
  client.connect(CLIENT_NAME, HA_USR, HA_PWD);
  client.publish(ha_topic, ha_payload);
  client.disconnect();
}

//  Check the crawlspace on the DS18B20 sensor (code from Lib examples)

double getTemp() {  
  float _temp;
  float fahrenheit = 0;
  int   i = 0;

  do {  _temp = sensor.getTemperature();
  } while (!sensor.crcCheck() && MAXRETRY > i++);
  if (i < MAXRETRY) {  
    fahrenheit = sensor.convertToFahrenheit(_temp);
  } else {
    Particle.publish("ERROR", "Invalid reading", PRIVATE);
  }
  return (fahrenheit); 
}

