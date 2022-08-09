// Variables

// Wiring
#define sensorPin       A0        // our pressure sensor reports a range from 0.5 to 4.5v

// reporting and checking intervals
#define CHECK        10000        // check the pump pressure frequently
#define REPORT      299993        // report every 5 min (why not use a prime to avoid colliding with check)
#define ALARM        60000        // if in danger, sound the alarm every minute!
#define DOGTIME     120000        // wait 2 minutes before putting the paddles on

// sensor vars
float pressure    = 00.00;        // sensor readings from 0.5-4.5v
// danger tripwire settings
float dangerHigh  = 50.00;        // above this threshold maybe someone closed all valves
float dangerLow   = 35.00;        // below this threshold we probably lost our prime
bool  inDanger      = FALSE;      // start with a clean slate!

// mqtt debugging
int   mqttCt        = 0;
int   mqttFails     = 0;        
int   mqttDis       = 0;        // unexpected disconnects

retained bool SELF_RESTART = FALSE;
int fails = 0;
int GIVE_UP = 3;