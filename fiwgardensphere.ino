//
//
//      Fresno Ideaworks GardenSphere
//      Monitor and Control 
//
//      v 0.1.4 19 Aug 2017
//      jdozeran@gmail.com
//
//      Updated to include relays for grow lights, hydroponics recirculating pump,
//      and water pump for drip irrigation.
//      Also includes functions to read soil moisture from 2 probes and report it as
//      the average of the two readings.
//      Temperature (in farenheight and celsius), humidity, heat index and dew point
//      are queryable from outside the unit.


#include "Adafruit_DHT_Particle.h"

#define DHTPIN D2       // what pin we're connected to
#define DHTTYPE DHT22		// DHT 22 (AM2302)

// Connect pin 1 (on the left) of the sensor to +5V
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

DHT dht(DHTPIN, DHTTYPE);
int loopCount;
double h, t, f, hi, dp, k;
int sm;

int LightsRelay = D1;   // Pin to turn on and off grow lights
int PumpRelay = D6;     // Pin to turn on and off water pump.
int HydroRelay = D5;    // Pin to turn on and off nutrient solution pump.
int MoistEnable1 = D3;   // Pin used to anable moisture sensor so it doesn't delaminate
int MoistPin1 = A1;      // Pin connected to soil moisture sensor
int MoistValue1 = 0;     // Initial moisture value for soil - reads 0 - 4095
int MoistEnable2 = D4;   // Pin used to anable moisture sensor so it doesn't delaminate
int MoistPin2 = A2;      // Pin connected to soil moisture sensor
int MoistValue2 = 0;     // Initial moisture value for soil - reads 0 - 4095


void setup() {
    Particle.variable("humid", &h, DOUBLE);
    Particle.variable("celcius", &t, DOUBLE);
    Particle.variable("faren", &f, DOUBLE);
    Particle.variable("heatindex", &hi, DOUBLE);
    Particle.variable("dewpoint", &dp, DOUBLE);
    Particle.variable("kelvin", &k, DOUBLE);
    Particle.variable("soilmoisture", &sm, INT);
    Serial.begin(9600); 
	Serial.println("Fresno Ideaworks GardenSphere!");
	Particle.publish("state", "GardenSphere start");
	Particle.function("readsoil",readSoil);
    Particle.function("lightsOnOff",lightsToggle);
    Particle.function("pumpOnOff",pumpToggle);
    Particle.function("hydroOnOff",nutrientToggle);
	dht.begin();
	pinMode(MoistEnable1,OUTPUT);
	digitalWrite(MoistEnable1,0);
	pinMode(MoistEnable2,OUTPUT);
	digitalWrite(MoistEnable2,0);
	pinMode(PumpRelay,OUTPUT);
	digitalWrite(PumpRelay,1);
	pinMode(LightsRelay,OUTPUT);
	digitalWrite(LightsRelay,1);
	pinMode(HydroRelay,OUTPUT);
	digitalWrite(HydroRelay,1);
	loopCount = 0;
	delay(2000);
}

void loop() {
// Reading temperature or humidity takes about 250 milliseconds!
// Sensor readings may also be up to 2 seconds 'old' (its a 
// very slow sensor)
	 h = dht.getHumidity();
// Read temperature as Celsius
	 t = dht.getTempCelcius();
// Rad temperature as Farenheit
	 f = dht.getTempFarenheit();
  
// Check if any reads failed and exit early (to try again).
	if (isnan(h) || isnan(t) || isnan(f)) {
		Serial.println("Failed to read from DHT sensor!");
	    Particle.publish("state", "Failed to read from DHT sensor!");
		return;
	}
	else {
	    Particle.publish("state", "Read success from DHT sensor!");
	}

// Compute heat index
// Must send in temp in Fahrenheit!
	hi = dht.getHeatIndex();
	dp = dht.getDewPoint();
	k = dht.getTempKelvin();
    
    readSoil("internal");
	//String timeStamp = Time.timeStr();
	Particle.publish("readings", String::format("{\"Hum(\%)\": %4.2f, \"Temp(°C)\": %4.2f, \"DP(°C)\": %4.2f, \"HI(°C)\": %4.2f}", h, t, dp, hi));
	delay(600000); //10 minutes between readings.
	//delay(9000);
	loopCount++;
}

int readSoil(String command)
{
  if(command.toUpperCase() == "INTERNAL")
  {
    digitalWrite(MoistEnable1,1);
    delay(1000);
    MoistValue1 = analogRead(MoistPin1);
    digitalWrite(MoistEnable1,0);
    digitalWrite(MoistEnable2,1);
    delay(1000);
    MoistValue2 = analogRead(MoistPin2);
    digitalWrite(MoistEnable2,0);
    sm = (MoistValue1+MoistValue2) / 2;
    Particle.publish("state", "Read soil moisture sensor");
    return sm;
  }
  else if (command.toUpperCase() == "")
  {
    digitalWrite(MoistEnable1,1);
    delay(1000);
    MoistValue1 = analogRead(MoistPin1);
    digitalWrite(MoistEnable1,0);
    digitalWrite(MoistEnable2,1);
    delay(1000);
    MoistValue2 = analogRead(MoistPin2);
    digitalWrite(MoistEnable2,0);
    sm = (MoistValue1+MoistValue2) / 2;
    Particle.publish("state", "Read soil moisture sensor");
    return sm;
  }
  else return -1;
}

int lightsToggle(String command)
{
  if(command.toUpperCase() == "ON")
  {
    digitalWrite(LightsRelay,0);
    Particle.publish("state", "Grow lights turned on.");
    return 1;
  }
  else if (command.toUpperCase() == "OFF")
  {
    digitalWrite(LightsRelay,1);
    Particle.publish("state", "Grow lights turned off.");
    return 1;
  }
  else return -1;
}

int pumpToggle(String command)
{
  if(command.toUpperCase() == "ON")
  {
    digitalWrite(PumpRelay,0);
    Particle.publish("state", "Water pump on for soil");
    return 1;
  }
  else if (command.toUpperCase() == "OFF")
  {
    digitalWrite(PumpRelay,1);
    Particle.publish("state", "Water pump off for soil");
    return 1;
  }
  else return -1;
}


int nutrientToggle(String command)
{
  if(command.toUpperCase() == "ON")
  {
    digitalWrite(HydroRelay,0);
    Particle.publish("state", "Nutrient recirculating pump on");
    return 1;
  }
  else if (command.toUpperCase() == "OFF")
  {
    digitalWrite(HydroRelay,1);
    Particle.publish("state", "Nutrient recirculating pump off");
    return 1;
  }
  else return -1;
}
