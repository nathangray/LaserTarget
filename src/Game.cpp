
#ifndef Game_h
#include "Game.h"
#endif

Game::Game(void)
{
	state = State::IDLE;
	init();
}

void Game::init() {
	Serial.printf("Initted [%s]\n", getType().c_str());
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
}
