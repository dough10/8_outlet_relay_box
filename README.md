# 8_outlet_relay_box V:1.0.0

Wemos D1 mini controlled 8 relay board w/ acs712 for power reading

## install

1. clone repo `git clone https://github.com/dough10/8_outlet_relay_box.git`
2. cd into directory `cd 8_outlet_relay_box`
3. modify config.h with your wifi credentials
4. modify defined RELAY namespaces in config.h to fit your project pin numbers
5. flash 8_outlet_relay_box.ino on Wemos D1 Mini

## usage

- "ip address" returns device states
- "ip address/1" toggles relay1 (1 - 8) also returns device states
- "ip address/1/on" turn on relay1 (1 - 8) also returns device states
- "ip address/1/off" turn off relay1 (1 - 8) also returns device states
