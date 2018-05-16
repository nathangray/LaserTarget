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
#ifndef Node_h
#include <Node.h>
#endif

#define NODE_COUNT 5
#define GAME_TICK 1000 // How fast the game 'ticks' in ms

class Game {
public:
	enum class State : int {IDLE=0, STARTING=1, PLAY=2, ENDING=10, END=11};

	Game(std::vector<Node> &_nodes);
	virtual ~Game(){}

	virtual void init();
	virtual void start();
	virtual inline String getType()
	{
		return "NONE";
	}
	virtual State getState();
	virtual void setState(int _state);
	virtual void setState(State _state);
	virtual void getStatus(JsonObject& game);

	virtual void shot(Node &node, int team_id, int damage);
	virtual void tick();

protected:
	State state = State::IDLE;
	std::vector<Node> &nodes;
	Team* winner = NULL;
	Team teams[TEAM_COUNT_MAX] = {
		Team(0,Team::BLUE),
		Team(1,Team::RED),
		Team(2,Team::GREEN),
		Team(3,Team::WHITE)
	};
	H4_TIMER timer;
};


 #endif
