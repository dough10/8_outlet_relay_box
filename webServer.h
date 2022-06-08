#ifndef webServer_h
#define webServer_h

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "power.h"
#include "relay.h"
#include "config.h"

const char* ssid = STASSID;
const char* password = STAPSK;

Relay Relays[] = {
  RELAY1,
  RELAY2,
  RELAY3,
  RELAY4,
  RELAY5,
  RELAY6,
  RELAY7,
  RELAY8,
};

Power power;

ESP8266WebServer server(80);

class webServer {
  public:

    void init() {
      Serial.begin(115200);
      Serial.println("");
      Serial.println(__FILE__);
      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid, password);
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
      Serial.println("");
      Serial.print("Connected to ");
      Serial.println(ssid);
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
      if (MDNS.begin("esp8266")) {
        Serial.println("MDNS responder started");
      }
      server.on("/", states);
      for (int i = 0; i < num_relays; i++) {
        String name = String(i + 1);
        Relays[i].init();
        Serial.println("Relay " + name + " initalized.");
        server.on("/" + name, [i]() {
          Relays[i].toggle();
          states();
        });
        server.on("/" + name + "/on", [i]() {
          Relays[i].on();
          states();
        });
        server.on("/" + name + "/off", [i]() {
          Relays[i].off();
          states();
        });
      }
      server.onNotFound(fourohfour);
      server.begin();
      Serial.println("HTTP server started");
      power.init();
    };

    void update() {
      server.handleClient();
      MDNS.update();
      power.takeReading();
    }


  private:
    static const int num_relays = sizeof(Relays);

    static String json() {
      StaticJsonDocument<384> doc;
      for (int i = 0; i < num_relays; i++) {
        doc[String(i + 1)] = !digitalRead(Relays[i].pin);
      }
      doc["amps"] = power.amps();
      doc["watts"] = power.watts();
      doc["kWh"] = power.kWh();
      doc["uptime"] = power.uptime; 
      doc["network"] = ssid;
      doc["address"] = WiFi.localIP();
      String output;
      serializeJson(doc, output);
      return output;
    }

    static void states() {
      server.send(200, "application/json", json());
    }

    static void fourohfour() {
      String message = "File Not Found\n\n";
      message += "URI: ";
      message += server.uri();
      message += "\nMethod: ";
      message += (server.method() == HTTP_GET) ? "GET" : "POST";
      message += "\nArguments: ";
      message += server.args();
      message += "\n";
      for (unsigned long i = 0; i < server.args(); i++) {
        message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
      }
      server.send(404, "text/plain", message);
    }
};

#endif
