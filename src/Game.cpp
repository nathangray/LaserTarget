
#ifndef Game_h
#include "Game.h"
#endif

Game::Game(std::vector<Node> &_nodes)
{
	state = State::IDLE;
	nodes = _nodes;
	init();
}

void Game::init() {
	for( auto &node: nodes)
	{
		node.setState(Node::State::IDLE);
	}
}

void Game::start() {
	state = State::STARTING;
}

void Game::setState(State _state)
{
	// Set specifically to always be Idle for this base game
	state = State::IDLE;
}
Game::State Game::getState()
{
	return state;
}

void Game::getStatus(JsonObject& game)
{
	game["type"] = getType();
	if(winner != NULL) {
		game["winner"] = winner->id;
	}
	JsonArray& teamList = game.createNestedArray("teams");
	for(int i = 0; i < TEAM_COUNT_MAX; i++)
	{
		JsonObject& teamStatus = teamList.createNestedObject();
		teamStatus["id"] = teams[i].id;
		teamStatus["score"] = teams[i].score;
	}
}
