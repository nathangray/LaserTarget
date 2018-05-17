

#ifndef Lights_h
#define Lights_h

#ifndef Arduino_h
#include "Arduino.h"
#endif
#ifndef Team_h
#include <Team.h>
#endif

// Timer
#include <H4.h>
#include<functional>

// Blinkenlight strips
// Neopixel strip must be plugged into RX pin (RXD0 / GPIO3)
#include <ws2812_i2s.h>
#define LED_COUNT 30

void setupLEDs();

void hit(int team);
void showScore(int team, int score);
void blackout();
void idle();
void starting();
void play();

void fill_solid(  Pixel_t * leds, int numToFill, Pixel_t& color);
uint8_t scoreToLed(int score);
Pixel_t teamColor(int team);

// Animations
void ripple(Pixel_t hue, int height);
int wrap(byte top_led, int step);

// Single frame
void _hit(Pixel_t color, int &frame);
void _ripple(int &direction, int &frame) ;
#endif
