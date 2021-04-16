#ifndef Controller_h
#define Controller_h

#include "Arduino.h"

class Controller
{
public:
    Controller(int pinUp, int pinDown, int pinStop, int pinLock);
    void up();
    void down();
    void stop();
    void lock();

private:
    int _pinUp, _pinDown, _pinStop, _pinLock;
};

#endif