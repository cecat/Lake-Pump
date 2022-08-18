// Variables

// Wiring
#define sensorPin       A0        // our pressure sensor reports a range from 0.5 to 4.5v

// reporting and checking intervals
#define CHECK             3001        // check the pump pressure frequently ~3s intervals
#define REPORT           19997        // report average to HASS every ~20s

// sensor vars
double pressure    =       0.0;        // sensor readings from 0.5-4.5v
double cache       =       0.0;        // temp cache to hold reading before converting w/ map()
double averagePSI  =       0.0;        // let's report the average of the past few readings
double psiRecent[] =                   // just the past 6, which are spaced CHECK ms apart
    {40.0,40.0,40.0,40.0,40.0,40.0};   // all 40 which is normal pressure so we don't trip a fault during startup
int        psiPtr  =         0;        // index through psiRecent
double        acc  =       0.0;        // accumulator for averging

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

// to debug
int sequence = 0;
double seq0[] =                   // simulate normal
    {42.0,41.0,44.0,40.0,53.0,40.0};
double seq1[] =                   // simulate increasing
    {45.0,48.0,50.0,55.0,58.0,60.0};
double seq2[] =                   // simulate decreasing
    {39.0,32.0,34.0,30.0,25.0,20.0};   