
// All nodes
#include <ArduinoOTA.h>
#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#endif
#ifdef ARDUINO_ARCH_ESP32
#include <WiFi.h>
#endif

// Root / Server
#include <FS.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

// Client / Leaf
#include <ESP8266HTTPClient.h>
#include <WebSocketsClient.h>

// Timer
#include <H4.h>

#ifndef Lights_h
#include "Lights.h"
#endif

// IR Receiver
#include "IR.h"

// Game
#include <Game.h>
#include <Game/Domination.h>
#include <Node.h>

// Webserver stuff
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

WebSocketsClient webSocket;

// Network
const char* ssid = "Gray";
const char* password = "nathangray";
const char* hostName = "LaserTarget";
const char* http_username = "admin";
const char* http_password = "admin";

// Node
enum Role : byte {ROOT, LEAF};
Role role;

// Game
#define NODE_COUNT 5 // Including the "server" node
#define NODE_TIMEOUT 15000 // 15 seconds
#define NODE_UPDATE_SPEED 10000 // Every second we tell the nodes what's up
std::vector<Node> nodes;
Game* game = new Game(nodes);

// Timers
H4 h4;
H4_TIMER node_update;

/**
 * Get a node from the list by its ID
 */
Node& getNode(uint32_t id) {
	Serial.print("Looking for "); Serial.println(id, HEX);
	uint i = 0;
	for(; i < nodes.size(); i++) {
		if(id == nodes[i].getID()) {
			Serial.printf("Node %d is %04x\n", i, id);
			return nodes[i];
		}
			Serial.printf("Node %d is %x\n", i, nodes[i].getID());
	}
	nodes.push_back(Node(id));
	return nodes[i];
}

/**
 * Get the current game status
 */
String getGameStatus()
{
	StaticJsonBuffer<1024> jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();
	root["state"] = (int)game->getState();
	JsonObject& gamestatus = root.createNestedObject("game");
	game->getStatus(gamestatus);

	JsonArray& node_object = root.createNestedArray("nodes");
	for(uint node_idx = 0; node_idx < nodes.size(); node_idx++) {
		Serial.printf("Getting status for node #%d [%x]\n", node_idx, nodes[node_idx].getID());
		nodes[node_idx].getStatus(node_object.createNestedObject());
	}

	String json;
	root.printTo(json);
	return json;
}

void setGameState(int state)
{
	Serial.printf("%s setGameState(%d)\n", game->getType().c_str(), state);
	game->setState(state);
	ws.textAll(getGameStatus());

	Serial.printf("%s getState()=%d\n", game->getType().c_str(), (int)game->getState());
	switch(game->getState())
	{
		case Game::State::PLAY:
			node_update = h4.every(GAME_TICK, []() {
				ws.textAll(getGameStatus());
				// End if no longer playing
				if(game->getState() == Game::State::END) h4.never(node_update);
			});
			break;
		case Game::State::IDLE:
		case Game::State::END:
			h4.never(node_update);
	}
}

/**
 * Process a game JSON object from client
 */
void processGame(JsonObject& game_info)
{
	String type = game_info.get<String>("type");
	String old_type = game->getType();
	if(game->getState() == Game::State::IDLE)
	{
		if(type == "DOMINATION") {
			game = new Domination(nodes);
		} else {
			game = new Game(nodes);
		}

		// If game type changed, notify all
		if(old_type != game->getType())
		{
			ws.textAll(getGameStatus());
		}
	}
	Serial.printf("Created %s\n", game->getType().c_str());
}

/**
 * Server processes a message from a node
 */
void processNodeMsg(JsonObject& msg, AsyncWebSocketClient* client)
{
		Node &node = getNode(strtoul(msg.get<char*>("node"), NULL, 0));
		node.setClient(client);
		node.updateTimestamp();
		JsonVariant hit = msg["hit"];
		if(hit.success()) {
			game->shot(node, hit.as<int>(), msg["damage"]);
		}
}
/**
 * Process a websocket message from a client - either node or browser
 */
void processWebsocket(JsonObject& msg, AsyncWebSocketClient * client)
{
	JsonVariant node_id = msg["node"];
	if(node_id.success()) {
		Serial.printf("Got socket msg from node %s\n", node_id.as<char*>());
		processNodeMsg(msg, client);
	}
	JsonVariant state = msg["state"];
	if(state.success())
	{
		setGameState(msg.get<int>("state"));
	}
	JsonVariant game = msg.get<JsonVariant>("game");
	if(game.success())
	{
		processGame(game.as<JsonObject>());
	}
}

/**
 * Process a websocket message from the server on a leaf node
 */
void processWebsocket(JsonObject& msg) {
	Node& node = nodes[0];
	JsonVariant state = msg["state"];
	if(state.success())
	{
		node.setState(msg.get<int>("state"));
	}
}

/**
 * Handle websocket events (as server)
 */
void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  if(type == WS_EVT_CONNECT){
    Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
		client->text(getGameStatus());
  } else if(type == WS_EVT_DISCONNECT){
    Serial.printf("ws[%s][%u] disconnect: %u\n", server->url(), client->id());
  } else if(type == WS_EVT_ERROR){
    Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
  } else if(type == WS_EVT_PONG){
    Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len)?(char*)data:"");
  } else if(type == WS_EVT_DATA){
    AwsFrameInfo * info = (AwsFrameInfo*)arg;
    String msg = "";
		StaticJsonBuffer<200> jsonBuffer;
    if(info->final && info->index == 0 && info->len == len){
      //the whole message is in a single frame and we got all of it's data
      Serial.printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT)?"text":"binary", info->len);

      if(info->opcode == WS_TEXT){
        for(size_t i=0; i < info->len; i++) {
          msg += (char) data[i];
        }
      }
      Serial.printf("%s\n",msg.c_str());

      if(info->opcode == WS_TEXT) {
				JsonObject& root = jsonBuffer.parseObject(msg);
        if(root.success())
				{
					processWebsocket(root, client);
				}
				else
				{
					Serial.printf("Unable to parse JSON:\n%s\n", msg.c_str());
				}
			}
    } else {
      //message is comprised of multiple frames or the frame is split into multiple packets
      if(info->index == 0){
        if(info->num == 0)
          Serial.printf("ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
        Serial.printf("ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
      }

      Serial.printf("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT)?"text":"binary", info->index, info->index + len);

      if(info->opcode == WS_TEXT){
        for(size_t i=0; i < info->len; i++) {
          msg += (char) data[i];
        }
      } else {
        char buff[3];
        for(size_t i=0; i < info->len; i++) {
          sprintf(buff, "%02x ", (uint8_t) data[i]);
          msg += buff ;
        }
      }
      Serial.printf("%s\n",msg.c_str());

      if((info->index + len) == info->len){
        Serial.printf("ws[%s][%u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);
        if(info->final){
          Serial.printf("ws[%s][%u] %s-message end\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
          if(info->message_opcode == WS_TEXT)
            client->text("I got your text message");
          else
            client->binary("I got your binary message");
        }
      }
    }
  }
}

/**
 * Leaf node handling WebSocket events from server
 */
void clientWsEvent(WStype_t type, uint8_t * payload, size_t length) {

	StaticJsonBuffer<200> jsonBuffer;
	JsonObject& root = jsonBuffer.parseObject(payload);
	switch(type) {
		case WStype_DISCONNECTED:
			Serial.printf("[WSc] Disconnected!\n");
			break;
		case WStype_CONNECTED:
			Serial.printf("[WSc] Connected to url: %s\n", payload);

			// send message to server when Connected
			char stringBuffer16[16];
			sprintf(stringBuffer16,"{node:%04x}", ESP.getChipId());
			webSocket.sendTXT(stringBuffer16);
			break;
		case WStype_TEXT:
			Serial.printf("[WSc] get text: %s\n", payload);
			if(root.success())
			{
				processWebsocket(root);
			}
			else
			{
				Serial.printf("Unable to parse JSON:\n%s\n", payload);
			}
			// message came from server
			// webSocket.sendTXT("message here");
			break;
		case WStype_BIN:
			Serial.printf("[WSc] get binary length: %u\n", length);
			hexdump(payload, length);

			// message came from server
			// send data to server
			// webSocket.sendBIN(payload, length);
			break;
	}
}

/**
 * Check all clients to see if they've timed out
 */
 void clientTimeout() {
	for(uint i = 1; i < nodes.size(); i++) {
		if(millis() - nodes[i].getTimestamp() > NODE_TIMEOUT) {
			Serial.printf("Node %x timed out", nodes[i].getID());
			nodes[i].getClient()->close(0, NULL);
			nodes.erase(nodes.begin() + i);
			return; // just avoid reindex problems
		}
	}
 }

/**
 * Connect and set up as a simple leaf node
 */
void connectLeaf()
{
	role = LEAF;
	WiFi.mode(WIFI_STA);

	WiFi.begin(hostName);
	if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.printf("Connect failed!\n");
    WiFi.disconnect(false);
    delay(1000);
    WiFi.begin(hostName);
  }

	webSocket.begin("192.168.4.1", 80, "/ws");
	webSocket.onEvent(clientWsEvent);

	// try ever 5000 again if connection has failed
	webSocket.setReconnectInterval(5000);
}

/**
 * Connect and set up as the root (server)
 */
void connectRoot()
{
	role = ROOT;
	Serial.println("I'm the server");
	/*
	WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.printf("STA: Failed!\n");
    WiFi.disconnect(false);
    delay(1000);
    WiFi.begin(ssid, password);
  }
	if(!(uint32_t)WiFi.localIP())
	*/
	{
		WiFi.disconnect();
		WiFi.mode(WIFI_AP);
	}
  WiFi.softAP(hostName);

	MDNS.begin(hostName);
	MDNS.addService("http","tcp",80);

  SPIFFS.begin();

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", String(ESP.getFreeHeap()));
  });
	server.on("/ip", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", String(WiFi.localIP() ? WiFi.localIP().toString() : WiFi.softAPIP().toString()));

    Serial.println(String(WiFi.localIP() ? WiFi.localIP().toString() : WiFi.softAPIP().toString()));
  });

	// Get game state
	server.on("/state", HTTP_GET, [](AsyncWebServerRequest *request){
		request->send(200, "text/plain", String((int)game->getState()));
	});

  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

  server.onNotFound([](AsyncWebServerRequest *request){
    Serial.printf("NOT_FOUND: ");
    if(request->method() == HTTP_GET)
      Serial.printf("GET");
    else if(request->method() == HTTP_POST)
      Serial.printf("POST");
    else if(request->method() == HTTP_DELETE)
      Serial.printf("DELETE");
    else if(request->method() == HTTP_PUT)
      Serial.printf("PUT");
    else if(request->method() == HTTP_PATCH)
      Serial.printf("PATCH");
    else if(request->method() == HTTP_HEAD)
      Serial.printf("HEAD");
    else if(request->method() == HTTP_OPTIONS)
      Serial.printf("OPTIONS");
    else
      Serial.printf("UNKNOWN");
    Serial.printf(" http://%s%s\n", request->host().c_str(), request->url().c_str());

    if(request->contentLength()){
      Serial.printf("_CONTENT_TYPE: %s\n", request->contentType().c_str());
      Serial.printf("_CONTENT_LENGTH: %u\n", request->contentLength());
    }

    int headers = request->headers();
    int i;
    for(i=0;i<headers;i++){
      AsyncWebHeader* h = request->getHeader(i);
      Serial.printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
    }

    int params = request->params();
    for(i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if(p->isFile()){
        Serial.printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
      } else if(p->isPost()){
        Serial.printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
      } else {
        Serial.printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
      }
    }

    request->send(404);
  });
  server.onFileUpload([](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final){
    if(!index)
      Serial.printf("UploadStart: %s\n", filename.c_str());
    Serial.printf("%s", (const char*)data);
    if(final)
      Serial.printf("UploadEnd: %s (%u)\n", filename.c_str(), index+len);
  });
  server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
    if(!index)
      Serial.printf("BodyStart: %u\n", total);
    Serial.printf("%s", (const char*)data);
    if(index + len == total)
      Serial.printf("BodyEnd: %u\n", total);
  });
  server.begin();

	// Start client timeout
	h4.every(NODE_TIMEOUT,clientTimeout);
}

/**
 * Set up the WiFi and determine if we're the server or just a client
 */
void connect() {
	WiFi.disconnect();
	nodes.clear();

	// Root acts as a leaf too
	nodes.push_back(Node(ESP.getChipId()));

	// Check to see if node network is up
	int n = WiFi.scanNetworks();
	for(int i = 0; i < n; i++)
	{
		if(WiFi.SSID(i) == hostName)
		{
			// Already there
			return connectLeaf();
		}
	}

	connectRoot();
}

/**
 * This node has been shot, the game decides what happens
 */
void hit(int team_id, int damage) {
	hit(team_id);
	if(role == LEAF) {
		char buffer[16];
		sprintf(buffer, "{node:%04x,hit:%d,damage:%d}",ESP.getChipId(), team_id, damage);
		webSocket.sendTXT(buffer);
	} else {
		game->shot(nodes[0], team_id, damage);
		Serial.printf("Current owner: %d\n", nodes[0].getOwner()->id);
	}
}

void setup(){
	delay(5000);
  Serial.begin(115200);
  Serial.setDebugOutput(true);
	Serial.println("Hello");
	setupLEDs();
	connect();
	setupIR();
}

void loop(){
	loopIR();
	if(role == LEAF)
	{
	  webSocket.loop();

		// Ping server
		static unsigned long last = 0;
	  if(abs(millis() - last) > 5000) {
			char stringBuffer80[80];
			sprintf(stringBuffer80,"{node:%04x, tick:%d}", ESP.getChipId(), millis());
    	webSocket.sendTXT(stringBuffer80);
	    last = millis();
	  }
	}
	else
	{
		h4.loop();
	}
}
