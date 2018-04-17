/**
 * Game base class
 */

#ifndef Game_h
#define Game_h

#include "Arduino.h"
#include <ArduinoJson.h>

#ifndef Team_h
#include <Team.h>
#endif

class Game {
public:
	enum class State : int {IDLE=0, STARTING=1, PLAY=2, ENDING=10, END=11};
	Game();
	virtual ~Game(){}

	virtual void init();
	virtual void start();
	virtual inline String getType()
	{
		return "NONE";
	}
	virtual State getState();
	virtual inline void setState(int _state) {
		state = static_cast<State>(_state);
	}

	virtual void setState(State _state);
	virtual void getStatus(JsonObject& game);

protected:
	State state = State::IDLE;
	Team* teams;
};


 #endif
