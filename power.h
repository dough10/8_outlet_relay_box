#ifndef power_h
#define power_h

#include <ACS712.h>

ACS712  ACS(A0, 5.0, 1023, 100);

class Power {
  public:
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
      if (startTime - lastRan < looptime) {
        return;
      }
      read(startTime);
      lastRan = startTime;
    }

    float amps() {
      return _amps;
    }

    float watts() {
      return _watts;
    }

    float kWh() {
      return _kWh;
    }

    unsigned long uptime() {
      return _uptime;
    }

  private:
    static const int numReadings = 5;
    int readings[numReadings];
    int readIndex = 0;    
    long pool = 0; 
    unsigned long total = 0;
    const long looptime = 5000;
    unsigned long lastRan = millis();
    float _amps;
    float _watts;
    float _kWh; 
    unsigned long _uptime = 0;

    void read(unsigned long mills) {
      _uptime = mills;
      double hours = _uptime / 1000.0 /60.0 /60.0;
      pool = pool - readings[readIndex];
      readings[readIndex] = ACS.mA_AC();
      pool = pool + readings[readIndex];
      readIndex = readIndex + 1;
      if (readIndex >= numReadings) {
        readIndex = 0;
      }
      float mA = pool / numReadings;
      _amps = mA / 1000;
      _watts = _amps * 120.0;
      total += _watts;
      _kWh = (total * hours) / 1000;
      if (!Serial) {
        return;
      }
      Serial.println();

      Serial.print("last mA reading: ");
      Serial.println(readings[readIndex]);

      Serial.print("uptime: ");
      Serial.println(uptime());

      Serial.print("hours: ");
      Serial.println(hours,3);

      Serial.print("mA: ");
      Serial.println(mA,3);

      Serial.print("amps: ");
      Serial.println(amps(),3);

      Serial.print("watts: ");
      Serial.println(watts(),3);

      Serial.print("kWh: ");
      Serial.println(kWh(),3);
    }
};

#endif
