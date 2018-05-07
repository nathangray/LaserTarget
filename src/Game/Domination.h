/**
 * Domination game
 */

#include "Arduino.h"

#ifndef Domination_h
#define Domination_h

#ifndef Game_h
#include "Game.h"
#endif

#ifndef Team_h
#include <Team.h>
#endif

class Domination : public Game {
public:

	Domination(std::vector<Node> &_nodes) : Game(_nodes) {};
	virtual void init();
	inline String getType() {
		return "DOMINATION";
	}
	inline void setState(State _state) {
		return Game::setState((int)_state);
	}
	//JsonObject& getStatus();
};

#endif
