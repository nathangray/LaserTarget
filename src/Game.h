/**
 * Game base class
 */

#ifndef Game_h
#define Game_h

#ifndef Team_h
#include <Team.h>
#endif

class Game {
public:
	Game();

	void init();
	void start();
	int getStatus();

private:
	Team* teams;
};

 #endif
