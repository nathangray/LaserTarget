
#ifndef Game_h
#include <Game.h>
#endif

Game::Game(void)
{}
void Game::setState(int _state)
{
	state = _state;
}
int Game::getState()
{
	return state;
}

String Game::getStatus()
{
	return "{}";
}
