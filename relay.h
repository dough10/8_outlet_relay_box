#ifndef relay_h
#define relay_h

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

#endif