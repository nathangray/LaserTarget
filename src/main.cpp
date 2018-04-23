
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

// Game
#include <H4.h>
#include <Game.h>
#include <Game/Domination.h>

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
Game* game = new Game();

/**
 * Get the current game status
 */
String getGameStatus()
{
	StaticJsonBuffer<200> jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();
	root["state"] = (int)game->getState();
	JsonObject& gamestatus = root.createNestedObject("game");
	game->getStatus(gamestatus);

	String json;
	root.printTo(json);
	return json;
}

void setGameState(int state)
{
	game->setState(state);
	ws.textAll(getGameStatus());
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
		delete game;
		if(type == "DOMINATION") {
			game = new Domination();
		} else {
			game = new Game();
		}
		// If game type changed, notify all
		if(old_type != game->getType())
		{
			ws.textAll(getGameStatus());
		}
	}
}

/**
 * Process a websocket message from a browser
 */
void processWebsocket(JsonObject& msg)
{
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
					processWebsocket(root);
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

void clientWsEvent(WStype_t type, uint8_t * payload, size_t length) {

	switch(type) {
		case WStype_DISCONNECTED:
			Serial.printf("[WSc] Disconnected!\n");
			break;
		case WStype_CONNECTED: {
			Serial.printf("[WSc] Connected to url: %s\n", payload);

			// send message to server when Connected
			char stringBuffer16[16];
			sprintf(stringBuffer16,"{node:%04x}", ESP.getChipId());
			webSocket.sendTXT(stringBuffer16);
		}
			break;
		case WStype_TEXT:
			Serial.printf("[WSc] get text: %s\n", payload);

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
 * Connect and set up as a simple leaf node
 */
void connectLeaf()
{
	role = LEAF;

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
	WiFi.disconnect();
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

	Serial.print("Soft-AP IP: "); Serial.println(WiFi.softAPIP());
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

}

/**
 * Set up the WiFi and determine if we're the server or just a client
 */
void connect() {
	// Root acts as a leaf too

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

void setup(){
  Serial.begin(115200);
  Serial.setDebugOutput(true);
	connect();

}

void loop(){
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
}
