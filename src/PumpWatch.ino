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

void setup() {
    //for debug- function to enter test pressure values
    Particle.function("testPSI", newPressure);

    Time.zone (-5);
    Particle.syncTime();

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
  delay(1000);
// check everything when timer fires; notify only state changes
    if (TimeToCheck) {
        TimeToCheck = FALSE;

        //cache = analogRead(sensorPin);
        //testing instead of above analogRead while the sensor is in the mail
          cache = random(500, 4500)/1000.0; 

        pressure = map(cache, 0.5, 4.5, 0.0, 100.0);

        // evaluate pressure
        if (pressure > dangerLow && pressure < dangerHigh) {
          if (inDanger) {
            tellHASS(Topics[0], String(pressure));
            tellHASS(Topics[3], String(pressure));
            inDanger=FALSE;
          } 
        }
        if (pressure < dangerLow)    { 
          //delay(MULLIGAN);    // let's give it time to see if this is a temporary thing
          //cache = analogRead(sensorPin);
          //pressure = map(cache, 0.5, 4.5, 0.0, 100.0);
          if (pressure < dangerLow) {  // still low?
            tellHASS(Topics[1], String(pressure)); 
            tellHASS(Topics[3], String(pressure));
            inDanger=TRUE;
          }
        }
        if (pressure > dangerHigh)    { 
          //delay(MULLIGAN);    // let's give it time to see if this is a temporary thing
          //cache = analogRead(sensorPin);
          //pressure = map(cache, 0.5, 4.5, 0.0, 100.0);
          if (pressure > dangerHigh) { // still high?
            tellHASS(Topics[2], String(pressure)); 
            tellHASS(Topics[3], String(pressure));
            inDanger=TRUE;
          }
        }
    }

    if (TimeToReport) {
      TimeToReport = FALSE;

      //client.disconnect();
      if (!client.isConnected()) {
        client.connect(CLIENT_NAME, HA_USR,HA_PWD);
        delay(2000);
      }

      if (client.isConnected()){
        fails=0;
        tellHASS(Topics[3], String(pressure));
        if (inDanger){
          if (pressure < dangerLow) {
              tellHASS(Topics[1], String(pressure));
            }
            else { tellHASS(Topics[2], String(pressure));}
          } else   tellHASS(Topics[0], String(pressure));
        //client.disconnect();
      } else {
        Particle.publish("mqtt", "Failed  connect", 3600, PRIVATE);
        mqttFails++;
        fails++;
        if (fails > GIVE_UP) {
          SELF_RESTART=TRUE;
          Particle.publish("-STUCK-", "Too many fails- restarting", 3600, PRIVATE);
          System.reset(RESET_NO_WAIT);
        }
      }
      Particle.publish("MQTT", String("Fail rate " + String(mqttFails) + "/" + String(mqttCt)),3600, PRIVATE);
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

int newPressure(String command)
{
  pressure = 40;
  if (command == "high") pressure = 60;
  if (command == "low") pressure = 30;
  TimeToReport = TRUE;
  return 1;
}
