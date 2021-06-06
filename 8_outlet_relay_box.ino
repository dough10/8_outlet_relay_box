#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ACS712.h>
#include "config.h"

#ifndef RELAY1
  #define RELAY1 2
  #define RELAY2 0
  #define RELAY3 5
  #define RELAY4 4
  #define RELAY5 16
  #define RELAY6 14
  #define RELAY7 12
  #define RELAY8 13
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

ESP8266WebServer server(80);

ACS712  ACS(A0, 3.3, 1023, 100);

const int numReadings = 5;

class Power {
  public:
    unsigned long uptime = 0;
    float amps;
    float watts;
    float kWh; 

    void init() {
      for (int thisReading = 0; thisReading < numReadings; thisReading++) {
        readings[thisReading] = 0;
      }
      ACS.autoMidPoint();
      Serial.print("MidPoint: ");
      Serial.print(ACS.getMidPoint());
      Serial.print(". Noise mV: ");
      Serial.println(ACS.getNoisemV());
    }

    void read() {
      unsigned long startTime = millis();
      uptime = millis() - bootTime;
      if (startTime - lastRan < looptime) {
        return;
      }
      double hours = uptime / 1000.0 /60.0 /60.0;
      pool = pool - readings[readIndex];
      readings[readIndex] = ACS.mA_AC();
      pool = pool + readings[readIndex];
      readIndex = readIndex + 1;
      if (readIndex >= numReadings) {
        readIndex = 0;
      }
      float mA = pool / numReadings;
      amps = mA / 1000;
      watts = amps * 120.0;
      total += watts;
      kWh = (total * hours) / 1000;
      if (Serial) {
        Serial.println(bootTime);
        Serial.println();

        Serial.print("uptime: ");
        Serial.println(uptime);

        Serial.print("hours: ");
        Serial.println(hours,3);

        Serial.print("mA: ");
        Serial.println(mA,3);

        Serial.print("amps: ");
        Serial.println(amps,3);

        Serial.print("watts: ");
        Serial.println(watts,3);

        Serial.print("kWh: ");
        Serial.println(kWh,3);
      }
      lastRan = startTime;
    }

  private:
    const unsigned long bootTime = millis();
    int readings[numReadings];
    int readIndex = 0;    
    long pool = 0; 
    unsigned long total = 0;
    const long looptime = 1000;
    unsigned long lastRan = bootTime - looptime;
};

Power power;

class Relay {
  public:
    uint8_t pin;

    void init() {
      pinMode(pin, OUTPUT);
      off();
    }

    void on() {
      digitalWrite(pin, 0);
    }

    void off() {
      digitalWrite(pin, 1);
    }

    void toggle() {
      digitalWrite(pin, !digitalRead(pin));
    }
};

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

const int num_relays = sizeof(Relays);

void states() {
  StaticJsonDocument<212> doc;
  for (int i = 0; i < num_relays; i++) {
    doc[String(i + 1)] = !digitalRead(Relays[i].pin);
  }
  doc["amps"] = power.amps;
  doc["watts"] = power.watts;
  doc["kWh"] = power.kWh;
  doc["uptime"] = power.uptime;
  String output;
  serializeJson(doc, output);
  server.send(200, "application/json", output);
}

void fourohfour() {
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

void setup(void) {
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

  power.init();

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  MDNS.update();
  power.read();
}
