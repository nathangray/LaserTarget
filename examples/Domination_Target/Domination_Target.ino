

/**
Domination / Timer for Laser tag

Copyright (c) 2017 Nathan Gray

There are 4 teams, each with a timer.  Some blinkenlights too.

States:
SET -> Play -> Win -> Idle -> Sleep

On power on we start, hardcoded for 2 minutes

When the game starts,
this thing is neutral.  As soon as one team presses their button, 
their timer starts.  If the other team presses their button, the 
first team's timer stops, and the second team's timer starts.  First
team to the set time for the game wins.

** NOT COMPLETE Aug 2017**
*  ******* Pull D3 to ground to program

*/

// For interrupt based timing
#include <Metro.h>

// Blinkenlight strips
// Neopixel strip must be plugged into RX pin (RXD0 / GPIO3)
#include <ws2812_i2s.h>

// Store time
#include <EEPROM.h>

#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
/*
 *System defines
 */
#define LED_COUNT 30
#define TEAM_COUNT 4
#define MAX_SCORE 2*60*60 // Max 2 hour
// Whoever is not in control is dimmed
#define NON_SEGMENT_BRIGHTNESS 4 // For 7 seg, 0-15
#define NON_LED_BRIGHTNESS 60    // For LEDs, 0-255

#define ANIMATE_INTERVAL 50
#define SCORE_ADDRESS 0

// System states
#define STATE_SET  0
#define STATE_PLAY 1
#define STATE_WIN  2
#define STATE_IDLE 3

#define BLUE_TEAM 0
#define RED_TEAM 1
#define GREEN_TEAM 2
#define WHITE_TEAM 3

#define IR_PIN 2
// IR Receiver
IRrecv irrecv(IR_PIN);
decode_results  results;        // Somewhere to store the results

// Set up lights
WS2812 ledstrip;
uint8_t esp_brightness;
static Pixel_t black[LED_COUNT];
  
// Structure for teams
struct Team {
  int score;
  byte top_led;

  Pixel_t hue;
  Pixel_t leds[LED_COUNT];
};

Team teams[] = {
   // Colors in GRB
  {0, 0, {0, 0, 0xFF}},
  {0, 0, {0, 0xFF, 0}},
  {0, 0, {0xFF, 0, 0}},
  {0, 0, {0xFF, 0xFF, 0xFF}},
};

Metro game_timer = Metro(1000);
Metro led_animate = Metro(ANIMATE_INTERVAL);

// State variables
byte state = STATE_PLAY;
int target_score = 2*60; // 2 minutes
Team* owner = NULL;              // Who's owning

// Light animation
int led_step = 0;
int center = 0;
#define FADE_STEPS 16
uint8_t fade[] = {254,245,219,179,131,83,41,13,2,11,37,77,128,173,215,243};
//{128,173,215,243,254,245,219,179,131,83,41,13,2,11,37,77};
Pixel_t Black = {0,0,0};

void setup() {
  Serial.begin(115200);

  ledstrip.init(LED_COUNT);
  
  // Start the receiver
  irrecv.enableIRIn();  
  
  // Load stored time
  int read = EEPROM.read(SCORE_ADDRESS);
  if(read != 255 && read != 0)
  {
    target_score = 60 * read;
  }
  // Show some life
  for(int i = 0xFF; i >= 0; i-= 1)
  {
    for(int j = 0; j < LED_COUNT; j++)
    {
      black[j] = {i, i, i};
    }
    ledstrip.show(black);
    yield();
    delay(10);
  }
  
  // More LED
  for(int j = 0; j < LED_COUNT; j++)
  {
    black[j] = Black;
    for(byte i = 0; i < TEAM_COUNT; i++)
    {
      teams[i].leds[j] = Black;
    }
    delay(25);
  }
  ledstrip.show(black);
  Serial.println("Hello");
 
}

void loop() {
  Serial.print(".");
  switch(state)
  {
    //case STATE_SET: set(); break;
    case STATE_PLAY: play(); break;
  case STATE_WIN: win(); break;
    case STATE_IDLE: idle(); break;
  }
  
}

// This keeps everything ticking
void loop_checks()
{
  if(led_animate.check())
  {
    if(owner != NULL)
    {
      ripple(owner);
      owner->leds[owner->top_led] = owner->hue;
      leds_show(owner->leds);
    }
  }
  
}

void leds_show(Pixel_t leds[])
{
  ledstrip.show(leds);
}
/**
 * Just after power-on, we'd like to change the setup
 *
void set() {
  // Start in set mode
  for(byte i = 0; i < TEAM_COUNT; i++)
  {
    teams[i].display.blinkRate(2);
  }

  while (state == STATE_SET)
  {
    loop_checks();
    
    // Done setup -> go to PLAY
    if(!teams[RED_TEAM].bounce.read() && !teams[BLUE_TEAM].bounce.read())
    {
      state = STATE_PLAY;
      
      return;
    }
      
    if(teams[RED_TEAM].bounce.fell() && digitalRead(teams[BLUE_TEAM].button_pin))
    {
      target_score -= 60;
    }
    if(teams[BLUE_TEAM].bounce.fell() && digitalRead(teams[RED_TEAM].button_pin))
    {
      target_score += 60;
    }
    target_score = constrain(target_score, 60, MAX_SCORE);
    // Set displays directly
    for(byte i = 0; i < TEAM_COUNT; i++)
    {
      show_time(teams[i].display, target_score);
    }
    
    // Update EEPROM
    int read = EEPROM.read(SCORE_ADDRESS);
    if(read * 60 != target_score)
    {
      EEPROM.write(SCORE_ADDRESS, target_score / 60);
    }
  }
}
*/

/**
 * Time to play
 */
void play() {  
  
  for(byte i = 0; i < TEAM_COUNT; i++)
  {
    // More LED
    for(int j = 0; j < LED_COUNT; j++)
    {
      teams[i].leds[j] = Black;
    }
  }
  leds_show(black);
  
  while (state == STATE_PLAY)
  {
    loop_checks();
  
    if (irrecv.decode(&results)) {  // Grab an IR code

      Serial.print((long)results.value, HEX);
      irrecv.resume();              // Prepare for the next value
      int t = (results.value >> (8*2)) & 0xff;
      if(t < 1 || t > TEAM_COUNT)
      {
        continue;
      }
      if(owner != &teams[t-1])
      {
        owner =& teams[t-1];
        Serial.print("Owner now "); Serial.println(t-1);
        ledstrip.show(black);
        
        // Dim
        if(owner != NULL)
        {
          Pixel_t dimmed = owner->hue;
          dimmed.R *= 0.8;
          dimmed.G *= 0.8;
          dimmed.B *= 0.8;
          fill_solid(&(owner->leds[0]), owner->top_led, dimmed);
        }
       
        // Reset LED at the bottom to prevent wrong score showing
        led_step = 0;
      }
    }
    
    if(game_timer.check())
    {
      if(owner != NULL)
      {
        owner->score++;
        owner->top_led = (owner->score * LED_COUNT) / target_score;
        // Gradually speed up animation as we get close
        led_animate.interval(ANIMATE_INTERVAL);
        int percent = owner->score * 100 / target_score;
        if(percent > 85)
        {
          led_animate.interval(map(percent, 85, 100, ANIMATE_INTERVAL, 5));
        }
        if(owner->score >= target_score)
        {
          state = STATE_WIN;
          return;
        }
      }
    }
    
    yield();
  }
  
}

/**
 * Yay, someone won
 */
void win() {
  for(int j = LED_COUNT-1; j >= 0 ; j--)
  {
    for(byte i = 0; i < TEAM_COUNT; i++)
    {
      teams[i].leds[j] = owner->hue;
    }
    leds_show(owner->leds);
    delay(25);
  }
  delay(10000);
  state = STATE_IDLE;
}

/**
 * Nothing happening, look cool
 */
void idle() {
  led_animate.interval(2*ANIMATE_INTERVAL);
  //FastLED.setBrightness(NON_LED_BRIGHTNESS);
  for(byte i = 0; i < TEAM_COUNT; i++)
  {
    fill_solid(teams[i].leds, owner->top_led, owner->hue);
    // Need to set top LED so ripple works
    teams[i].top_led = owner->top_led;
  }
  
  while (state == STATE_IDLE)
  {
    yield();
    if(led_animate.check())
    {
      for(byte i = 0; i < TEAM_COUNT; i++)
      {
        ripple(owner,-1, owner->hue);
          
        leds_show(owner->leds);
        
        if (irrecv.decode(&results)) {  // Grab an IR code
          irrecv.resume();  
          state = STATE_PLAY;
        }
      }   
    }
  }
  
  // Reset some stuff, just in case
  for(byte i = 0; i < TEAM_COUNT; i++)
  {
    teams[i].score = 0;
    teams[i].top_led = 0;
    fill_solid(teams[i].leds, LED_COUNT, Black);
  }
  //FastLED.setBrightness(255);
  owner = NULL;
}

void fill_solid(  Pixel_t * leds, int numToFill, Pixel_t& color)
{
  for( int i = 0; i < numToFill; i++) {
    leds[i] = color;
  }
}
void ripple(struct Team *t) {
  ripple(t, 1, t->hue);
}

// Direction should be 1 or -1
void ripple(struct Team *t, int direction, Pixel_t& hue) {
  if(t->top_led < 3) return;
  for(byte i = 0; i < FADE_STEPS && i < t->top_led; i++)
  {
    Pixel_t shaded = {min(hue.G, fade[i]), min(hue.R, fade[i]), min(hue.B, fade[i])};
    t->leds[wrap(t->top_led,i+led_step)] = shaded;
  }
  led_step = wrap(t->top_led,led_step + direction);
}

int wrap(byte top_led, int step) {
  if(step < 0) return top_led + step;
  if(step > top_led -1) return step - top_led ;
  return step;
} // wrap()

