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
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#undef ARDUINO

#define OLED_RESET 0
Adafruit_SSD1306 display(OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

#define LOGO16_GLCD_HEIGHT 16 
#define LOGO16_GLCD_WIDTH  16 
static const unsigned char PROGMEM logo16_glcd_bmp[] = {
	0b00000000, 0b11000000,
	0b00000001, 0b11000000,
	0b00000001, 0b11000000,
	0b00000011, 0b11100000,
	0b11110011, 0b11100000,
	0b11111110, 0b11111000,
	0b01111110, 0b11111111,
	0b00110011, 0b10011111,
	0b00011111, 0b11111100,
	0b00001101, 0b01110000,
	0b00011011, 0b10100000,
	0b00111111, 0b11100000,
	0b00111111, 0b11110000,
	0b01111100, 0b11110000,
	0b01110000, 0b01110000,
	0b00000000, 0b00110000
};

#if (SSD1306_LCDHEIGHT != 64)
	#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

// Prototypes
void testdrawbitmap(const uint8_t *bitmap, uint8_t w, uint8_t h);
void testdrawchar(void);
void testdrawcircle(void);
void testfillrect(void);
void testdrawtriangle(void);
void testfilltriangle(void);
void testdrawroundrect(void);
void testfillroundrect(void);
void testdrawrect(void);
void testdrawline();
void testscrolltext(void);

void setup() {
	// by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
	display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false);  // initialize with the I2C addr 0x3C (for the 128x64)
	// init done

	// Show image buffer on the display hardware.
	// Since the buffer is intialized with an Adafruit splashscreen
	// internally, this will display the splashscreen.
	display.display();
	delay(2000);

	// Clear the buffer.
	display.clearDisplay();

	// draw a single pixel
	display.drawPixel(10, 10, WHITE);
	// Show the display buffer on the hardware.
	// NOTE: You _must_ call display after making any drawing commands
	// to make them visible on the display hardware!
	display.display();
	delay(2000);
	display.clearDisplay();

	// draw many lines
	testdrawline();
	display.display();
	delay(2000);
	display.clearDisplay();

	// draw rectangles
	testdrawrect();
	display.display();
	delay(2000);
	display.clearDisplay();

	// draw multiple rectangles
	testfillrect();
	display.display();
	delay(2000);
	display.clearDisplay();

	// draw mulitple circles
	testdrawcircle();
	display.display();
	delay(2000);
	display.clearDisplay();

	// draw a white circle, 10 pixel radius
	display.fillCircle(display.width()/2, display.height()/2, 10, WHITE);
	display.display();
	delay(2000);
	display.clearDisplay();

	testdrawroundrect();
	delay(2000);
	display.clearDisplay();

	testfillroundrect();
	delay(2000);
	display.clearDisplay();

	testdrawtriangle();
	delay(2000);
	display.clearDisplay();

	testfilltriangle();
	delay(2000);
	display.clearDisplay();

	// draw the first ~12 characters in the font
	testdrawchar();
	display.display();
	delay(2000);
	display.clearDisplay();

	// draw scrolling text
	testscrolltext();
	delay(2000);
	display.clearDisplay();

	// text display tests
	display.setTextSize(1);
	display.setTextColor(WHITE);
	display.setCursor(0,0);
	display.println("Hello, world!");
	display.setTextColor(BLACK, WHITE); // 'inverted' text
	display.println(3.141592);
	display.setTextSize(2);
	display.setTextColor(WHITE);
	display.print("0x"); display.println(0xDEADBEEF, HEX);
	display.display();
	delay(2000);
	display.clearDisplay();

	// miniature bitmap display
	display.drawBitmap(30, 16,  logo16_glcd_bmp, 16, 16, 1);
	display.display();
	delay(1);

	// invert the display
	display.invertDisplay(true);
	delay(1000); 
	display.invertDisplay(false);
	delay(1000); 
	display.clearDisplay();

	// draw a bitmap icon and 'animate' movement
	testdrawbitmap(logo16_glcd_bmp, LOGO16_GLCD_HEIGHT, LOGO16_GLCD_WIDTH);
}

void presentation() {
	// Present locally attached sensors here    
}

void loop() {
	// Send locally attached sensors data here
}

void testdrawbitmap(const uint8_t *bitmap, uint8_t w, uint8_t h)
{
	uint8_t icons[NUMFLAKES][3];

	// initialize
	for (uint8_t f=0; f< NUMFLAKES; f++) {
		icons[f][XPOS] = random(display.width());
		icons[f][YPOS] = 0;
		icons[f][DELTAY] = random(5) + 1;

		debug("x: %d y: %d dy: %d\n", icons[f][XPOS], icons[f][YPOS], icons[f][DELTAY]);
	}

	while (1) {
		// draw each icon
		for (uint8_t f=0; f< NUMFLAKES; f++) {
			display.drawBitmap(icons[f][XPOS], icons[f][YPOS], bitmap, w, h, WHITE);
		}
		display.display();
		delay(200);

		// then erase it + move it
		for (uint8_t f=0; f< NUMFLAKES; f++) {
			display.drawBitmap(icons[f][XPOS], icons[f][YPOS], bitmap, w, h, BLACK);
			// move it
			icons[f][YPOS] += icons[f][DELTAY];
			// if its gone, reinit
			if (icons[f][YPOS] > display.height()) {
				icons[f][XPOS] = random(display.width());
				icons[f][YPOS] = 0;
				icons[f][DELTAY] = random(5) + 1;
			}
		}
	}
}

void testdrawchar(void)
{
	display.setTextSize(1);
	display.setTextColor(WHITE);
	display.setCursor(0,0);

	for (uint8_t i=0; i < 168; i++) {
		if (i == '\n') {
			continue;
		}
		display.write(i);
		if ((i > 0) && (i % 21 == 0)) {
			display.println();
		}
	}
	display.display();
	delay(1);
}

void testdrawcircle(void)
{
	for (int16_t i=0; i<display.height(); i+=2) {
		display.drawCircle(display.width()/2, display.height()/2, i, WHITE);
		display.display();
		delay(1);
	}
}

void testfillrect(void)
{
	uint8_t color = 1;

	for (int16_t i=0; i<display.height()/2; i+=3) {
		// alternate colors
		display.fillRect(i, i, display.width()-i*2, display.height()-i*2, color%2);
		display.display();
		delay(1);
		color++;
	}
}

void testdrawtriangle(void)
{
	for (int16_t i=0; i<min(display.width(),display.height())/2; i+=5) {
		display.drawTriangle(display.width()/2, display.height()/2-i,
							 display.width()/2-i, display.height()/2+i,
							 display.width()/2+i, display.height()/2+i, WHITE);
		display.display();
		delay(1);
	}
}

void testfilltriangle(void)
{
	uint8_t color = WHITE;

	for (int16_t i=min(display.width(),display.height())/2; i>0; i-=5) {
		display.fillTriangle(display.width()/2, display.height()/2-i,
							 display.width()/2-i, display.height()/2+i,
							 display.width()/2+i, display.height()/2+i, WHITE);
		if (color == WHITE) {
			color = BLACK;
		} else {
			color = WHITE;
		}
		display.display();
		delay(1);
	}
}

void testdrawroundrect(void)
{
	for (int16_t i=0; i<display.height()/2-2; i+=2) {
		display.drawRoundRect(i, i, display.width()-2*i, display.height()-2*i, display.height()/4, WHITE);
		display.display();
		delay(1);
	}
}

void testfillroundrect(void)
{
	uint8_t color = WHITE;

	for (int16_t i=0; i<display.height()/2-2; i+=2) {
		display.fillRoundRect(i, i, display.width()-2*i, display.height()-2*i, display.height()/4, color);
		if (color == WHITE) {
			color = BLACK;
		} else {
			color = WHITE;
		}
		display.display();
		delay(1);
	}
}

void testdrawrect(void)
{
	for (int16_t i=0; i<display.height()/2; i+=2) {
		display.drawRect(i, i, display.width()-2*i, display.height()-2*i, WHITE);
		display.display();
		delay(1);
	}
}

void testdrawline()
{
	for (int16_t i=0; i<display.width(); i+=4) {
		display.drawLine(0, 0, i, display.height()-1, WHITE);
		display.display();
		delay(1);
	}
	for (int16_t i=0; i<display.height(); i+=4) {
		display.drawLine(0, 0, display.width()-1, i, WHITE);
		display.display();
		delay(1);
	}
	delay(250);

	display.clearDisplay();
	for (int16_t i=0; i<display.width(); i+=4) {
		display.drawLine(0, display.height()-1, i, 0, WHITE);
		display.display();
		delay(1);
	}
	for (int16_t i=display.height()-1; i>=0; i-=4) {
		display.drawLine(0, display.height()-1, display.width()-1, i, WHITE);
		display.display();
		delay(1);
	}
	delay(250);

	display.clearDisplay();
	for (int16_t i=display.width()-1; i>=0; i-=4) {
		display.drawLine(display.width()-1, display.height()-1, i, 0, WHITE);
		display.display();
		delay(1);
	}
	for (int16_t i=display.height()-1; i>=0; i-=4) {
		display.drawLine(display.width()-1, display.height()-1, 0, i, WHITE);
		display.display();
		delay(1);
	}
	delay(250);

	display.clearDisplay();
	for (int16_t i=0; i<display.height(); i+=4) {
		display.drawLine(display.width()-1, 0, 0, i, WHITE);
		display.display();
		delay(1);
	}
	for (int16_t i=0; i<display.width(); i+=4) {
		display.drawLine(display.width()-1, 0, i, display.height()-1, WHITE);
		display.display();
		delay(1);
	}
	delay(250);
}

void testscrolltext(void)
{
	display.setTextSize(2);
	display.setTextColor(WHITE);
	display.setCursor(10,0);
	display.clearDisplay();
	display.println("scroll");
	display.display();
	delay(1);

	display.startscrollright(0x00, 0x0F);
	delay(2000);
	display.stopscroll();
	delay(1000);
	display.startscrollleft(0x00, 0x0F);
	delay(2000);
	display.stopscroll();
	delay(1000);    
	display.startscrolldiagright(0x00, 0x07);
	delay(2000);
	display.startscrolldiagleft(0x00, 0x07);
	delay(2000);
	display.stopscroll();
}
