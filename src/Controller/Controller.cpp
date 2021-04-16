#include "Arduino.h"
#include "Controller.h"

Controller::Controller(int pinUp, int pinDown, int pinStop, int pinLock)
{
    pinMode(pinUp, OUTPUT);
    pinMode(pinDown, OUTPUT);
    pinMode(pinStop, OUTPUT);
    pinMode(pinLock, OUTPUT);
    _pinUp = pinUp;
    _pinDown = pinDown;
    _pinStop = pinStop;
    _pinLock = pinLock;
}
void Controller::up()
{
    digitalWrite(_pinUp, HIGH);
    delay(1000);
    digitalWrite(_pinUp, LOW);
}
void Controller::down()
{
    digitalWrite(_pinDown, HIGH);
    delay(1000);
    digitalWrite(_pinDown, LOW);
}
void Controller::stop()
{
    digitalWrite(_pinStop, HIGH);
    delay(1000);
    digitalWrite(_pinStop, LOW);
}
void Controller::lock()
{
    digitalWrite(_pinLock, HIGH);
    delay(1000);
    digitalWrite(_pinLock, LOW);
}