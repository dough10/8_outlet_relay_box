#ifndef power_h
#define power_h

#include <ACS712.h>

const int numReadings = 5;

ACS712  ACS(A0, 3.3, 1023, 100);

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

    void takeReading() {
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
      lastRan = startTime;
      if (!Serial) {
        return;
      }
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

  private:
    const unsigned long bootTime = millis();
    int readings[numReadings];
    int readIndex = 0;    
    long pool = 0; 
    unsigned long total = 0;
    const long looptime = 5000;
    unsigned long lastRan = bootTime - looptime;
};

#endif
