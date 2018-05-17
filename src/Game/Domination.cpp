
#ifndef Domination_h
#include "Domination.h"
#endif

// Should probably be configurable
#define MAX_SCORE 120

void Domination::init() {
	Game::init();

	winner = NULL;
	for(uint8_t i = 0; i < TEAM_COUNT_MAX; i++) {
		teams[i].score = 0;
	}
}
void Domination::setState(State _state) {
	state = _state;
	Serial.printf("%s getState()=%d\n", this->getType().c_str(), (int)this->getState());
	switch(state)
	{
		case State::IDLE:
			Domination::init();
			break;
		case State::STARTING:
			Domination::init();
			// Wait 10s, then start
			h4.once(10000, bind(&Domination::setState, this, State::PLAY));
			break;
		case State::PLAY:
			// Start game ticker
			timer = h4.every(GAME_TICK, bind(&Domination::tick, this));
			break;
		case State::ENDING:
			// Set all nodes to winner
			for( auto &node: nodes) {
				node.setOwner(winner->id);
				node.setTeamScore(winner->id, 100);
			}
			// Wait 10s, then end
			h4.once(10000, bind(&Domination::setState, this, State::END));
			// Fall through
		default:
			h4.never(timer);
			break;
	}
	for( auto &node: nodes)
 	{
		Serial.printf("Setting %x state %d\n", node.getID(), _state);
 		node.setState((int)_state);
 	}
}

void Domination::shot(Node &node, int team_id, int damage) {
	Serial.println("HERE - 1");
	Serial.flush();
	// We only care if we're actually playing
	if(state != State::PLAY) return;

	node.setOwner(team_id);
	Serial.printf("Node %04x got shot, owner now %d\n", node.getID(), node.getOwner()->id);
}

/**
 * Add the current number of owned nodes to each team's score, & update the
 * nodes
 */
void Domination::tick() {
	Serial.print(".");
	for( auto &node: nodes)
 	{
		if(node.getOwner()) {
			teams[node.getOwner()->id].score++;
			node.setTeamScore(node.getOwner()->id,
					(teams[node.getOwner()->id].score * 100) / MAX_SCORE);
		}
	}

	// Check for win conditions - one team score hits the limit
	for(uint8_t i = 0; i < TEAM_COUNT_MAX; i++) {
		if(teams[i].score >= MAX_SCORE) {
			this->winner =& teams[i];
			this->setState(State::ENDING);
		}
	}
}
