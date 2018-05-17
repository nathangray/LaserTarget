

#ifndef Lights_h
#include <Lights.h>
#endif

// Set up lights
WS2812 ledstrip;
Pixel_t Black   = {0,0,0};
Pixel_t DarkRed = {0, 0xa0, 0};
Pixel_t Orange  = {0x8a, 0xfe, 0x00};
Pixel_t leds[LED_COUNT];

#define ANIMATION_SPEED 50
#define FADE_STEPS 16
uint8_t fade[] = {254,245,219,179,131,83,41,13,2,11,37,77,128,173,215,243};

H4_TIMER score_timer;

// Some variables to make score animation a little nicer
int ripple_height;
int ripple_dir;
Pixel_t ripple_hue;

void fill_solid(  Pixel_t * leds, int numToFill, Pixel_t& color)
{
  for( int i = 0; i < numToFill; i++) {
    leds[i] = color;
  }
}
void setupLEDs()
{
	ledstrip.init(LED_COUNT);

	// Show some life
	Pixel_t color;
  for(int i = 0xFF; i >= 0; i-= 1)
  {
		color = {i,i,i};
		fill_solid(leds, LED_COUNT, color);
    ledstrip.show(leds);
    yield();
    delay(10);
  }
	fill_solid(leds, LED_COUNT, Black);
  ledstrip.show(leds);
	Serial.println("DOne setupLEDs");
}
/**
 * Node was hit, show some indication
 */
void hit(int team_id) {
	static H4_TIMER hit_timer;
	if(hit_timer) h4.never(hit_timer);
	int frame = 0;
	Pixel_t color = teamColor(team_id);
	Serial.printf("[Lights] Got hit by %d\n", team_id);
	hit_timer = h4.nTimes(15, 50, bind(_hit, color, frame) );
}

/**
 * Show the current score, which is given as a percentage
 */
void showScore(int team, int score) {
	Pixel_t color = teamColor(team);
	Serial.printf("[Lights] Score: %d: %d %d\n",team, score,(score * LED_COUNT) / 100);

	for( int i = scoreToLed(score); i < LED_COUNT; i++) {
    leds[i] = Black;
  }
	ledstrip.show(leds);
	ripple(color, scoreToLed(score));
}
void blackout()
{
	if(score_timer) {
		h4.never(score_timer);
		score_timer = 0;
	}
	fill_solid(leds, LED_COUNT, Black);
	ledstrip.show(leds);
}
void idle()
{
	Pixel_t blue = {0, 0, 0xFF};
	fill_solid(leds, LED_COUNT, blue);
	ledstrip.show(leds);
}
void starting()
{
	if(score_timer) {
		h4.never(score_timer);
		score_timer = 0;
	}
	uint8_t n=0;
	h4.once(n++*1000, bind(fill_solid, leds, LED_COUNT, DarkRed), []() {ledstrip.show(leds);});
	h4.once(n++*1000, blackout);
	h4.once(n++*1000, bind(fill_solid, leds, LED_COUNT, DarkRed), []() {ledstrip.show(leds);});
	h4.once(n++*1000, blackout);
	h4.once(n++*1000, bind(fill_solid, leds, LED_COUNT, DarkRed), []() {ledstrip.show(leds);});
	h4.once(n++*1000, blackout);
	h4.once(n++*1000, bind(fill_solid, leds, LED_COUNT, Orange), []() {ledstrip.show(leds);});
	h4.once(n++*1000, blackout);
	h4.once(n++*1000, bind(fill_solid, leds, LED_COUNT, Orange), []() {ledstrip.show(leds);});
}
void play()
{
	Pixel_t green = {0xFF, 0, 0};
	fill_solid(leds, LED_COUNT, green);
	ledstrip.show(leds);
	// wait 1/2 second, then blackout
	h4.once(1000, blackout);
}

Pixel_t teamColor(int team_id) {
	Pixel_t color = Black;
	switch(team_id)
	{
		case 0: color = Team::BLUE; break;
		case 1: color = Team::RED; break;
		case 2: color = Team::GREEN; break;
		case 3: color = Team::WHITE; break;
	}
	return color;
}

uint8_t scoreToLed(int score) {
	uint8_t led = (score * LED_COUNT) / 100;
	if(score > 0 && led < 1) led = 1;
	return led;
}
void ripple(Pixel_t hue, int height) {
	Serial.printf("[Lights] ripple to %d\n",height);
	ripple_height = min(LED_COUNT, height);
	ripple_dir = 1;
	ripple_hue = hue;
	static int frame = 0;
	if(!score_timer) {
		score_timer = h4.every(ANIMATION_SPEED, bind(_ripple, ripple_dir, frame) );
	}
}

/**
 * Animation functions that handle a single frame
 ************************************************/

void _hit(Pixel_t color, int &frame) {
	const uint8_t HIT_HEIGHT = 10;
	Pixel_t dimmed = color;
	for(int i = 0; i < frame; i++) {
		dimmed.R *= 0.8;
		dimmed.G *= 0.8;
		dimmed.B *= 0.8;
	}

	for( int i = 0; i < HIT_HEIGHT; i++) {
		leds[i] = dimmed;
		dimmed.R *= 0.8;
		dimmed.G *= 0.8;
		dimmed.B *= 0.8;
		ledstrip.show(leds);
		if(dimmed.R <= 10 && dimmed.G <= 10 && dimmed.B <= 10) break;
	}
	frame++;
}

 // Direction should be 1 or -1
void _ripple(int &direction, int &frame) {
	if(ripple_height < 2) {
		leds[0] = leds[1] = ripple_hue;
		ledstrip.show(leds);
		return;
	}
	for(byte i = 0; i < FADE_STEPS && i < ripple_height; i++)
	{
		Pixel_t shaded = {min(ripple_hue.G, fade[i]), min(ripple_hue.R, fade[i]), min(ripple_hue.B, fade[i])};
		leds[wrap(ripple_height,i+frame)] = shaded;
	}
	frame = wrap(ripple_height,frame + direction);
	ledstrip.show(leds);
}

int wrap(byte top_led, int step) {
	if(step < 0) return top_led + step;
	if(step > top_led -1) return step - top_led ;
	return step;
} // wrap()
