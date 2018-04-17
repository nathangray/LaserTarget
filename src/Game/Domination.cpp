
#ifndef Domination_h
#include "Domination.h"
#endif

void Domination::init() {
	Game::init();

	Serial.println("WTF?");
	Serial.println(Domination::getType());
}
/*
JsonObject& Domination::getStatus()
{
	JsonObject& status = Game::getStatus();
	status.prettyPrintTo(Serial);
	return status;
}
*/
