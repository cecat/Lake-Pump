# Lake-Pump (PumpWatch)
Use Particle.io Electron (cellular) w/ LiPo batter to Monitor well pump (used to drive sprinklers, hoses, etc. using lake water) to ensure that it does not lose prime (low pressure) and that it is not operating with no flow (high pressure).

When pump is off this does not operate (it's powered by the same power switch that Home Assistant (HASS) uses to operate the pump.

While the pump is running, check pressure every 10sand report to Home Assistant using MQTT every 10 minutes just to give HASS a heartbeat.  If pressure drops below 35 PSI or goes above 50PSI, send HASS the reading immediately along with a request to shut down.
  
To use this code you will need to set either the hostname or IP address of your HA system,
which for this code we assume is running the Mosquitto MQTT broker.
Also, when setting up Mosquitto MQTT as a HASS integration you will have created
an MQTT username and password - those will also need to be set.

The best way to do this is to set up a file "secrets.h" that looks like this if you are
using a hostname:

```
const char *HA_USR = "your mqtt username";
const char *HA_PWD = "your mqtt passwd";
// if you are using a hostname for your MQTT broker (HASS):
char MY_SERVER[] = "your.server.hostname"
// OR using your IP address w.x.y.z
// byte MY_SERVER[] = { w, x, y, z };
```

Alternatively, you can comment out the "#include secrets.h" line and fill this information in
following the instructions in the code.

You can take the .ino file from the src directory and paste it into the Particle.io web IDE,
make the above customizations (adding a secrets.h file if you go that route),
then add the libraries (see project.properties)
If you are reading this a long time from the last update and
having trouble it could be that something has changed in the libraries, so you
can check the version number in the project.properties file as well.

