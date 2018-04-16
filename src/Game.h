/**
 * Game base class
 */

#ifndef Game_h
#define Game_h

#include "Arduino.h"

#ifndef Team_h
#include <Team.h>
#endif

class Game {
public:
	Game();
	virtual ~Game(){}

	void init();
	void start();
	int getState();
	void setState(int state);

	String getStatus();

private:
	int state = 0;
	Team* teams;
};

 #endif
