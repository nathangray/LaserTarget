/**
 * Node base class
 */

#ifndef Node_h
#include "Node.h"
#endif

Node::Node(uint32_t _id) {
	id = _id;
}

void Node::init() {
	owner = NULL;
	for(int i = 0; i < TEAM_COUNT_MAX; i++)
	{
		setTeamScore(i,0);
	}
}
void Node::start() {}
void Node::setState(State _state) {
	state = _state;
	if(client != NULL)
	{
		client->printf("{state:%d}\n", state);
	}
	switch(state) {
		case State::IDLE: Serial.println("IDLE"); idle(); break;
		case State::STARTING: starting(); break;
		case State::PLAY:
			Serial.println("PLAY"); play();
			break;
	}
	Serial.printf("Node %x state set to %d", id, state);
}
void Node::getStatus(JsonObject& node) {
	node["id"] = id;
	if(owner != NULL) {
		node["owner"] = owner->id;
	}
	JsonArray& teamList = node.createNestedArray("teams");
	for(int i = 0; i < TEAM_COUNT_MAX; i++)
	{
		JsonObject& teamStatus = teamList.createNestedObject();
		teamStatus["id"] = teams[i].id;
		teamStatus["score"] = teams[i].score;
	}
}
void Node::setOwner(int team_id)
{
	if(team_id < 0 || team_id > TEAM_COUNT_MAX) return;
	owner =& teams[team_id];
	if(client != NULL) {
		client->printf("{owner:%d}\n", state);
	} else {
		// TODO: Show this somehow?
	}
}
/**
 * Set the score for a team.  The score should be a percent.
 */
void Node::setTeamScore(uint8_t team_id, uint8_t score) {
	teams[team_id].score = score;
	Serial.printf("Node %04x team %d = %d\n",id, team_id, teams[team_id].score);
	if(client != NULL) {
		client->printf("{team:%d,score:%d}\n", team_id, score);
	}	else if (owner && team_id == owner->id) {
		showScore(team_id, score);
	}
}
