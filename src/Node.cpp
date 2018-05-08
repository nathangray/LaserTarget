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
	for(int i = 0; i < TEAM_COUNT_MAX; i++)
	{
		teams[i].score = 0;
	}
}
void Node::start() {}
void Node::setState(State _state) {
	state = _state;
	if(client != NULL)
	{
		client->printf("{state:%d}", state);
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
