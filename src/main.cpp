#ifndef UNIT_TEST

#include <Arduino.h>
#include <ESP8266WiFi.h>	 // base WiFi
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>

#include <ArduinoOTA.h>		 // Wireless code updates

#include <ESP8266WebServer.h>	// Web server
#include <DNSServer.h>
#include <WiFiManager.h>   // Wifi config magic

#include <painlessMesh.h>

#define   LED             D1       // GPIO number of connected LED, ON ESP-12 IS GPIO2

#define   BLINK_PERIOD    3000 // milliseconds until cycle repeat
#define   BLINK_DURATION  100  // milliseconds LED is on for

#define   MESH_SSID       "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

painlessMesh  mesh;
bool calc_delay = false;
SimpleList<uint32_t> nodes;

// Task to blink the number of nodes
Task blinkNoNodes;
bool onFlag = false;


/**
 * Initialize WiFi connection, if possible
 */
boolean wifi_init() {
	WiFiManager wifiManager;
	wifiManager.setTimeout(60);
	//first parameter is name of access point, second is the password
	wifiManager.startConfigPortal("LaserTarget");
}

/**
 * Initialize over the air updates
 */
void OTA_init() {
	ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}

void mesh_setup()
{
	mesh.setDebugMsgTypes(ERROR | DEBUG | CONNECTION);  // set before init() so that you can see startup messages

  mesh.init(MESH_SSID, MESH_PASSWORD, MESH_PORT);
  //mesh.onReceive(&receivedCallback);
//  mesh.onNewConnection(&newConnectionCallback);
//  mesh.onChangedConnections(&changedConnectionCallback);
///  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
//  mesh.onNodeDelayReceived(&delayReceivedCallback);

  //mesh.scheduler.addTask( taskSendMessage );
  //taskSendMessage.enable() ;

  blinkNoNodes.set(BLINK_PERIOD, (mesh.getNodeList().size() + 1) * 2, []() {
      // If on, switch off, else switch on
      if (onFlag)
        onFlag = false;
      else
        onFlag = true;
      blinkNoNodes.delay(BLINK_DURATION);

      if (blinkNoNodes.isLastIteration()) {
        // Finished blinking. Reset task for next run
        // blink number of nodes (including this node) times
        blinkNoNodes.setIterations((mesh.getNodeList().size() + 1) * 2);
        // Calculate delay based on current mesh time and BLINK_PERIOD
        // This results in blinks between nodes being synced
        blinkNoNodes.enableDelayed(BLINK_PERIOD -
            (mesh.getNodeTime() % (BLINK_PERIOD*1000))/1000);
      }
  });
  mesh.scheduler.addTask(blinkNoNodes);
  blinkNoNodes.enable();

  randomSeed(analogRead(A0));
}

void setup() {
	delay(2000);
	Serial.begin(115200);
	Serial.println("Booting");

	wifi_init();
	//mesh_setup();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  mesh.update();
  digitalWrite(LED, !onFlag);
}


void sendMessage() {
  String msg = "Hello from node ";
  msg += mesh.getNodeId();
  msg += " myFreeMemory: " + String(ESP.getFreeHeap());
  msg += " noTasks: " + String(mesh.scheduler.size());
  bool error = mesh.sendBroadcast(msg);

  if (calc_delay) {
    SimpleList<uint32_t>::iterator node = nodes.begin();
    while (node != nodes.end()) {
      mesh.startDelayMeas(*node);
      node++;
    }
    calc_delay = false;
  }

  Serial.printf("Sending message: %s\n", msg.c_str());

}
#endif
