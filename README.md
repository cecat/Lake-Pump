# PowerWatch
Use Particle.io Electron (cellular) w/ LiPo batter to Monitor crawlspace temp and power status at a remote home (avoid pipes freezing, etc.)

In steady state (no power outage, crawspace not too cold) theand power status to 
a remote (or local) Home Assistant (HASS) via MQTT every hour.
It will send a warning if the temperature falls below 35F
and another warning if the temperature falls to 32F or below. 

At HASS you can then up automations based on the topics as well as monitoring
power status and temperature over time.
This code uses MQTT with QoS=0 (i.e., none) but this still seems reliable with 
decent cellular connectivity (or if you were using WiFi, then then you'd also 
need to back up your WiFi and Internet gear to be notified of a power outage).

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

