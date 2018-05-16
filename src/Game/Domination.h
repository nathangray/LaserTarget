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
	void setState(State _state);
	void shot(Node &node, int team_id, int damage);
	void tick();
	//JsonObject& getStatus();
};

#endif
