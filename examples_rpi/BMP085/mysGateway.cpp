/**
 * The MySensors Arduino library handles the wireless radio link and protocol
 * between your home built sensors/actuators and HA controller of choice.
 * The sensors forms a self healing radio network with optional repeaters. Each
 * repeater and gateway builds a routing tables in EEPROM which keeps track of the
 * network topology allowing messages to be routed to nodes.
 *
 * Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
 * Copyright (C) 2013-2016 Sensnology AB
 * Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors
 *
 * Documentation: http://www.mysensors.org
 * Support Forum: http://forum.mysensors.org
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 */
 
#include <iostream>
#include <cstdio>
#include <unistd.h>

// For more options run ./configure --help

// Config file
//#define MY_LINUX_CONFIG_FILE "/etc/mysensors.dat"

// How many clients should be able to connect to this gateway (default 1)
#define MY_GATEWAY_MAX_CLIENTS 10

// Serial config
// Enable this if you are using an Arduino connected to the USB
//#define MY_LINUX_SERIAL_PORT "/dev/ttyACM0"
// Enable this if you need to connect to a controller running on the same device
//#define MY_IS_SERIAL_PTY
// Choose a symlink name for the PTY device
//#define MY_LINUX_SERIAL_PTY "/dev/ttyMySensorsGateway"
// Grant access to the specified system group for the serial device
//#define MY_LINUX_SERIAL_GROUPNAME "tty"

// MQTT options
//#define MY_CONTROLLER_IP_ADDRESS 192, 168, 178, 68
//#define MY_PORT 1883
//#define MY_MQTT_CLIENT_ID "mysensors-1"
//#define MY_MQTT_PUBLISH_TOPIC_PREFIX "mygateway1-out"
//#define MY_MQTT_SUBSCRIBE_TOPIC_PREFIX "mygateway1-in"

// Enable these if your MQTT broker requires usenrame/password
//#define MY_MQTT_USER "username"
//#define MY_MQTT_PASSWORD "password"

// Flash leds on rx/tx/err
//#define MY_DEFAULT_ERR_LED_PIN 12  // Error LED pin
//#define MY_DEFAULT_RX_LED_PIN  16  // Receive LED pin
//#define MY_DEFAULT_TX_LED_PIN  18  // Transmit LED pin
// Inverse the blinking feature
//#define MY_WITH_LEDS_BLINKING_INVERSE

// Enable software signing
//#define MY_SIGNING_SOFT
// Enable signing related debug
//#define MY_DEBUG_VERBOSE_SIGNING
// Enable this to request signatures from nodes that in turn request signatures from gateway
//#define MY_SIGNING_REQUEST_SIGNATURES
// Enable this to have gateway require all nodes in the network to sign messages sent to it
// Note: MY_SIGNING_REQUEST_SIGNATURES must also be set
//#define MY_SIGNING_GW_REQUEST_SIGNATURES_FROM_ALL

// Enables RF24 encryption (all nodes and gateway must have this enabled, and all must be personalized with the same AES key)
//#define MY_RF24_ENABLE_ENCRYPTION

#include <MySensors.h>

#define ARDUINO 100
// Include Arduino libraries here
#include <Adafruit_BMP085.h>
#undef ARDUINO

// Adapted from MySensors/PressureSensor.ino
// For more information visit: https://www.mysensors.org/build/pressure

int sample(float pressure);

#define BARO_CHILD 0
#define TEMP_CHILD 1

const float ALTITUDE = 688; // <-- adapt this value to your own location's altitude.

const char *weather[] = { "stable", "sunny", "cloudy", "unstable", "thunderstorm", "unknown" };
enum FORECAST
{
	STABLE = 0,            // "Stable Weather Pattern"
	SUNNY = 1,            // "Slowly rising Good Weather", "Clear/Sunny "
	CLOUDY = 2,            // "Slowly falling L-Pressure ", "Cloudy/Rain "
	UNSTABLE = 3,        // "Quickly rising H-Press",     "Not Stable"
	THUNDERSTORM = 4,    // "Quickly falling L-Press",    "Thunderstorm"
	UNKNOWN = 5            // "Unknown (More Time needed)
};

Adafruit_BMP085 bmp = Adafruit_BMP085();      // Digital Pressure Sensor 
bool bmp085_enabled = false;

float lastPressure = -1;
float lastTemp = -1;
int lastForecast = -1;

const int LAST_SAMPLES_COUNT = 5;
float lastPressureSamples[LAST_SAMPLES_COUNT];

// this CONVERSION_FACTOR is used to convert from Pa to kPa in forecast algorithm
// get kPa/h be dividing hPa by 10 
#define CONVERSION_FACTOR (1.0/10.0)

int minuteCount = 0;
bool firstRound = true;
// average value is used in forecast algorithm.
float pressureAvg;
// average after 2 hours is used as reference value for the next iteration.
float pressureAvg2;

float dP_dt;
bool metric;
MyMessage tempMsg(TEMP_CHILD, V_TEMP);
MyMessage pressureMsg(BARO_CHILD, V_PRESSURE);
MyMessage forecastMsg(BARO_CHILD, V_FORECAST);

unsigned long timer = 0;

void setup() {
	if (!bmp.begin()) {
		debug("Could not find a valid BMP085 sensor, check wiring!");
	} else {
		bmp085_enabled = true;
		debug("Found BMP085 sensor!");
	}
	metric = getConfig().isMetric;
}

void presentation() {
	// Send the sketch version information to the gateway and Controller
	sendSketchInfo("Pressure Sensor", "1.1");

	// Register sensors to gw (they will be created as child devices)
	present(BARO_CHILD, S_BARO);
	present(TEMP_CHILD, S_TEMP);  
}

void loop() {
	if (bmp085_enabled) {
		// Every 1min
		if (millis() - timer > 60000UL) {
			timer = millis();

			float pressure = bmp.readSealevelPressure(ALTITUDE) / 100.0;
			float temperature = bmp.readTemperature();

			if (!metric) {
				// Convert to fahrenheit
				temperature = temperature * 9.0 / 5.0 + 32.0;
			}

			int forecast = sample(pressure);

			debug("Temperature = %.2f %s\n", temperature, metric ? " *C" : " *F");
			debug("Pressure = %.2f hPa\n", pressure);
			debug("Forecast = %s\n", weather[forecast]);

			if (temperature != lastTemp) {
				send(tempMsg.set(temperature, 1));
				lastTemp = temperature;
			}

			if (pressure != lastPressure) {
				send(pressureMsg.set(pressure, 0));
				lastPressure = pressure;
			}

			if (forecast != lastForecast) {
				send(forecastMsg.set(weather[forecast]));
				lastForecast = forecast;
			}
		}
	}
}

float getLastPressureSamplesAverage()
{
	float lastPressureSamplesAverage = 0;

	for (int i = 0; i < LAST_SAMPLES_COUNT; i++) {
		lastPressureSamplesAverage += lastPressureSamples[i];
	}
	lastPressureSamplesAverage /= LAST_SAMPLES_COUNT;

	return lastPressureSamplesAverage;
}

// Algorithm found here
// http://www.freescale.com/files/sensors/doc/app_note/AN3914.pdf
// Pressure in hPa -->  forecast done by calculating kPa/h
int sample(float pressure)
{
	// Calculate the average of the last n minutes.
	int index = minuteCount % LAST_SAMPLES_COUNT;
	lastPressureSamples[index] = pressure;

	minuteCount++;
	if (minuteCount > 185) {
		minuteCount = 6;
	}

	if (minuteCount == 5) {
		pressureAvg = getLastPressureSamplesAverage();
	} else if (minuteCount == 35) {
		float lastPressureAvg = getLastPressureSamplesAverage();
		float change = (lastPressureAvg - pressureAvg) * CONVERSION_FACTOR;
		if (firstRound) { // first time initial 3 hour
			dP_dt = change * 2; // note this is for t = 0.5hour
		} else {
			dP_dt = change / 1.5; // divide by 1.5 as this is the difference in time from 0 value.
		}
	} else if (minuteCount == 65) {
		float lastPressureAvg = getLastPressureSamplesAverage();
		float change = (lastPressureAvg - pressureAvg) * CONVERSION_FACTOR;
		if (firstRound) { //first time initial 3 hour
			dP_dt = change; //note this is for t = 1 hour
		} else {
			dP_dt = change / 2; //divide by 2 as this is the difference in time from 0 value
		}
	} else if (minuteCount == 95) {
		float lastPressureAvg = getLastPressureSamplesAverage();
		float change = (lastPressureAvg - pressureAvg) * CONVERSION_FACTOR;
		if (firstRound) { // first time initial 3 hour
			dP_dt = change / 1.5; // note this is for t = 1.5 hour
		} else {
			dP_dt = change / 2.5; // divide by 2.5 as this is the difference in time from 0 value
		}
	} else if (minuteCount == 125) {
		float lastPressureAvg = getLastPressureSamplesAverage();
		pressureAvg2 = lastPressureAvg; // store for later use.
		float change = (lastPressureAvg - pressureAvg) * CONVERSION_FACTOR;
		if (firstRound) { // first time initial 3 hour
			dP_dt = change / 2; // note this is for t = 2 hour
		} else {
			dP_dt = change / 3; // divide by 3 as this is the difference in time from 0 value
		}
	} else if (minuteCount == 155) {
		float lastPressureAvg = getLastPressureSamplesAverage();
		float change = (lastPressureAvg - pressureAvg) * CONVERSION_FACTOR;
		if (firstRound) { // first time initial 3 hour
			dP_dt = change / 2.5; // note this is for t = 2.5 hour
		} else {
			dP_dt = change / 3.5; // divide by 3.5 as this is the difference in time from 0 value
		}
	} else if (minuteCount == 185) {
		float lastPressureAvg = getLastPressureSamplesAverage();
		float change = (lastPressureAvg - pressureAvg) * CONVERSION_FACTOR;
		if (firstRound) { // first time initial 3 hour
			dP_dt = change / 3; // note this is for t = 3 hour
		} else {
			dP_dt = change / 4; // divide by 4 as this is the difference in time from 0 value
		}
		pressureAvg = pressureAvg2; // Equating the pressure at 0 to the pressure at 2 hour after 3 hours have past.
		firstRound = false; // flag to let you know that this is on the past 3 hour mark. Initialized to 0 outside main loop.
	}

	int forecast = UNKNOWN;
	if (minuteCount < 35 && firstRound) { //if time is less than 35 min on the first 3 hour interval.
		forecast = UNKNOWN;
	} else if (dP_dt < (-0.25))	{
		forecast = THUNDERSTORM;
	} else if (dP_dt > 0.25) {
		forecast = UNSTABLE;
	} else if ((dP_dt > (-0.25)) && (dP_dt < (-0.05))) {
		forecast = CLOUDY;
	} else if ((dP_dt > 0.05) && (dP_dt < 0.25)) {
		forecast = SUNNY;
	} else if ((dP_dt >(-0.05)) && (dP_dt < 0.05)) {
		forecast = STABLE;
	} else {
		forecast = UNKNOWN;
	}

	// uncomment when debugging
	//printf(F("Forecast at minute "));
	//printf(minuteCount);
	//printf(F(" dP/dt = "));
	//printf(dP_dt);
	//printf(F("kPa/h --> "));
	//printf(weather[forecast]);
	//printf("/n");

	return forecast;
}
