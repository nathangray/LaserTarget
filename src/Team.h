/**
 * Team definition
 */
#ifndef Team_h
#define Team_h

// Blinkenlight strips
// Neopixel strip must be plugged into RX pin (RXD0 / GPIO3)
// Included here for colors Pixel_t
#include <ws2812_i2s.h>

#define TEAM_COUNT_MAX 4

class Team {
	public:
		inline Team(int _id, Pixel_t color, int _score = 0) {
			id = _id;
			hue = color;
			score = _score;
		}
		Pixel_t hue;
		int score;
		int id;

		// Colors in GRB
		static constexpr Pixel_t BLUE  {0, 0, 0xFF};
		static constexpr Pixel_t RED   {0, 0xFF, 0};
		static constexpr Pixel_t GREEN {0xFF, 0, 0};
		static constexpr Pixel_t WHITE {0xA0, 0xA0, 0xA0};
};


#endif
