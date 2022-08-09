/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#line 1 "/Users/charlescatlett/CODE/Lake-Pump/src/PumpWatch.ino"
/*
  Monitor wate pressure when lake pump is operating
    C. Catlett Aug 2022
    The host Particle Electron will only be powered when the pump is powered, so
    when pressure is outside of safe operating parameters (see vars.h) it will notify
    HASS via MQTT every minute.  It will know that HASS has acted (to turn off the pump)
    only in its final thoughts as it's being powered down...
      - When operating properly, the pump water pressure will be 38-42 PSI
        - Lower pressure indicates that pump has lost prime
        - Higher pressure indicates insufficient (or no) flow (not goot to run
          the pump with no sprinklers or hoses on)        
      - Tell HASS via MQTT to turn off pump if either of these conditions are detected
      - Otherwise, just give HASS a reassuring thumb's up every 5 min or so
        (specifically, tell HomeAssistant to do so)              
 */

#include <Particle.h>
#include <MQTT.h>
#include "secrets.h" 

// include topics for mqtt
#include "topics.h"
// include misc variables
#include "vars.h"

// MQTT 

void mqtt_callback(char* topic, byte* payload, unsigned int length);
void watchdogHandler();
void setup();
void loop();
void checkPSI();
void reportPSI();
void tellHASS (const char *ha_topic, String ha_payload);
#line 28 "/Users/charlescatlett/CODE/Lake-Pump/src/PumpWatch.ino"
#define MQTT_KEEPALIVE 35 * 60              // 60s is std default
void timer_callback_send_mqqt_data();    
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
     char p[length + 1];
     memcpy(p, payload, length);
     p[length] = 0; 
     Particle.publish("mqtt recvd", p, 3600, PRIVATE);
 }
MQTT client(MY_SERVER, 1883, MQTT_KEEPALIVE, mqtt_callback);
int MQTT_CODE = 0;

Timer checkTimer(CHECK, checkPSI);
Timer reportTimer(REPORT, reportPSI);
bool  TimeToCheck     = TRUE;
bool  TimeToReport    = TRUE;

// Application watchdog - sometimes MQTT wedges things
retained bool REBORN  = FALSE;  // did app watchdog restart us?
ApplicationWatchdog *wd;
void watchdogHandler() {
  REBORN = TRUE;
  System.reset(RESET_NO_WAIT);
}

void setup() {
    Time.zone (-5);
    Particle.syncTime();

    wd = new ApplicationWatchdog(DOGTIME, watchdogHandler, 1536); // restart after DOGTIME sec no pulse
    if (REBORN) {
      Particle.publish("****WEDGED****", "app watchdog restart", 3600, PRIVATE);
      REBORN = FALSE;
    }
    if (SELF_RESTART) {
      Particle.publish("----STUCK----", "self-reboot after 4 mqtt fails", 3600, PRIVATE);
      SELF_RESTART = FALSE;
    }

    pressure = analogRead(sensorPin);
    client.connect(CLIENT_NAME, HA_USR, HA_PWD);
    // check MQTT 
    if (client.isConnected()) {
        Particle.publish("mqtt_startup", "Connected to HA", 3600, PRIVATE);
        //client.disconnect();
      } else {
        Particle.publish("mqtt_startup", "Fail connect HA - check secrets.h", 3600, PRIVATE);
    }
    //client.disconnect();
    checkTimer.start();
    reportTimer.start();
}

void loop() {

// check everything when timer fires; notify only state changes
    if (TimeToCheck) {
        TimeToCheck = FALSE;
        pressure = analogRead(sensorPin);

        // evaluate pressure
        if (pressure > dangerLow && pressure < dangerHigh) {
          if (inDanger) {
            tellHASS(TOPIC_A, String(pressure));
            inDanger=FALSE;
          }
        }
        if (pressure < dangerLow)    { 
          delay(30000);    // let's give it 30s to see if this is a temporary thing
          if (pressure < dangerLow) {
            tellHASS(TOPIC_B, String(pressure)); 
            inDanger=TRUE;
          }
        }
        if (pressure > dangerHigh)    { 
          delay(30000);    // let's give it 30s to see if this is a temporary thing
          if (pressure < dangerLow) {
            tellHASS(TOPIC_C, String(pressure)); 
            inDanger=TRUE;
          }
        }
    }

    if (TimeToReport) {
      TimeToReport = FALSE;
      wd->checkin(); // poke app watchdog we're going in...

      //client.disconnect();
      if (client.isConnected()) {
        //Particle.publish("mqtt", "connected ok", 3600, PRIVATE); delay(100);
      } else {  
        Particle.publish("mqtt", "reconnecting", 3600, PRIVATE); delay(100);
        client.connect(CLIENT_NAME, HA_USR,HA_PWD);
        delay(2000);
      }

      if (client.isConnected()){
        fails=0;
        tellHASS(TOPIC_A, String(pressure));
        //client.disconnect();
      } else {
        Particle.publish("mqtt", "Failed to connect", 3600, PRIVATE);
        mqttFails++;
        fails++;
        if (fails > GIVE_UP) {
          SELF_RESTART=TRUE;
          Particle.publish("-STUCK-", "Too many fails- restarting", 3600, PRIVATE);
          System.reset(RESET_NO_WAIT);
        }
      }
      Particle.publish("MQTT", String("Fail rate " + String(mqttFails) + "/" + String(mqttCt)),3600, PRIVATE);
      void myWatchdogHandler(void); // reset the dog
    }
} 
/************************************/
/***         FUNCTIONS       ***/
/************************************/

// Checking timer interrupt handler
void checkPSI() {  TimeToCheck = TRUE;  }

// Reporting timer interrupt handler
void reportPSI() {  TimeToReport = TRUE;  }

// Report to HASS via MQTT
void tellHASS (const char *ha_topic, String ha_payload) {  

  delay(100); // bit of delay in between successive messages
  if (client.isConnected()){
    client.publish(ha_topic, ha_payload);
    mqttCt++;
  } else {
    mqttFails++;
    Particle.publish("mqtt", "Connection dropped", 3600, PRIVATE);
  }
}


