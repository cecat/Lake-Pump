// Variables

// Wiring
#define sensorPin       A0        // our pressure sensor reports a range from 0.5 to 4.5v

// reporting and checking intervals
#define CHECK        15000        // check the pump pressure frequently
#define REPORT      120000        // (testing every 2m - restore to 299993 to report every 5 min (why not use a prime to avoid colliding with check)
#define ALARM        30000        // if in danger, sound the alarm 30 seconds!
#define MULLIGAN      10000        // wait 10s to see if the reading is still high/low

// sensor vars
double pressure    =       0.0;        // sensor readings from 0.5-4.5v
double cache       =       0.0;        // temp cache to hold reading before converting w/ map()

// danger tripwire settings
double dangerHigh  = 50.00;        // above this threshold maybe someone closed all valves
double dangerLow   = 35.00;        // below this threshold we probably lost our prime
bool  inDanger      = FALSE;      // start with a clean slate!

// mqtt debugging
int   mqttCt        = 0;
int   mqttFails     = 0;        
int   mqttDis       = 0;        // unexpected mqtt disconnects

retained bool SELF_RESTART = FALSE;
int fails = 0;
int GIVE_UP = 3;

// to debug, use function to input pressure values 
int newPressure(String newPSI);