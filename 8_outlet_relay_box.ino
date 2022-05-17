#include "webServer.h"

webServer host;

void setup(void) {
  host.init();
}

void loop(void) {
  host.update();
}
