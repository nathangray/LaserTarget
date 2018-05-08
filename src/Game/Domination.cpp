
#ifndef Domination_h
#include "Domination.h"
#endif

void Domination::init() {
	Game::init();

	Serial.println("WTF?");
	Serial.println(Domination::getType());
}
void Domination::setState(int _state) {
	Game::setState(_state);
	Serial.printf("Domination setting state to %d", _state);
	for( auto &node: nodes)
 	{
 		node.setState(_state);
 	}
}
/*
JsonObject& Domination::getStatus()
{
	JsonObject& status = Game::getStatus();
	status.prettyPrintTo(Serial);
	return status;
}
*/
