/*
 * LogicOut.cpp
 *
 *  Created on: 07 Jul 2015
 *      Author: Jurie
 */

#include <wiringPi.h>

#include "LogicOut.h"


LogicOut::LogicOut(int _pin, bool _activeHigh):	pin(_pin), activeHigh(_activeHigh)
{
	pinMode(this->pin, OUTPUT);
	digitalWrite(this->pin,this->activeHigh==this->activeState);
}

LogicOut::~LogicOut() {
	// TODO Auto-generated destructor stub
}

bool LogicOut::setActive(bool act)
{
	digitalWrite(this->pin,this->activeHigh==act);

	this->activeState = digitalRead(this->pin) == this->activeHigh;	//Reads what state is set

	return act == this->activeState;	//Returns true if state correctly set
}
bool LogicOut::getState()
{
	return this->activeState;
}

