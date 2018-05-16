

#ifndef Lights_h
#include <Lights.h>
#endif

// Set up lights
WS2812 ledstrip;
Pixel_t Black = {0,0,0};
Pixel_t leds[LED_COUNT];


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
	fill_solid(leds, (score * LED_COUNT) / 100, color);
}
void blackout()
{
	fill_solid(leds, LED_COUNT, Black);
	ledstrip.show(leds);
}
void idle()
{
	Pixel_t blue = {0, 0, 0xFF};
	fill_solid(leds, LED_COUNT, blue);
	ledstrip.show(leds);
}
void play()
{
	Pixel_t green = {0xFF, 0, 0};
	fill_solid(leds, LED_COUNT, green);
	ledstrip.show(leds);
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
