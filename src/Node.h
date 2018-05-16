/**
 * Node base class
 */

#ifndef Node_h
#define Node_h

#include "Arduino.h"
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>

#ifndef Team_h
#include <Team.h>
#endif

#ifndef Lights_h
#include "Lights.h"
#endif

class Node {
public:
	enum class State : int {IDLE=0, STARTING=1, PLAY=2, ENDING=10, END=11, ERROR=20};
	inline Node(char* _id) {
		id = strtoul(_id, NULL, 0);
	}
	Node(uint32_t _id);
	virtual ~Node(){}

	virtual void init();
	virtual void start();
	virtual inline void setState(int _state) {
		state = static_cast<State>(_state);
		Node::setState(state);
	}
	virtual void setState(State _state);
	virtual void getStatus(JsonObject& node);
	inline uint32_t getID() { return id; }
	inline void setClient(AsyncWebSocketClient* _client) { client = _client; }
	inline AsyncWebSocketClient* getClient() { return client; }
	inline void updateTimestamp() { timestamp = millis(); }
	inline long getTimestamp() { return timestamp;}
	virtual void setOwner(int team_id);
	inline Team* getOwner() { return owner; }

protected:
	uint32_t id;         // Chip ID
	State state = State::IDLE;
	long timestamp;      // Last we heard from it
	AsyncWebSocketClient* client = NULL; // Websocket client
	Team* owner = NULL;
	Team teams[TEAM_COUNT_MAX] = {
		Team(0,Team::BLUE),
		Team(1,Team::RED),
		Team(2,Team::GREEN),
		Team(3,Team::WHITE)
	};
};


 #endif
