/*
 * Pump.cpp
 *
 *  Created on: 07 Jul 2015
 *      Author: Jurie
 */

#include "Pump.h"

#include <wiringPiI2C.h>
#include "LogicOut.h"
#include "LogicSensor.h"

Pump::Pump(int _fullSpeed, int _dacID, int startPin, int runningPin, int errStatusPin): pumpFullSpeed(_fullSpeed),pumpSpeed(_fullSpeed) ,\
dacID(_dacID),  dac(wiringPiI2CSetup(_dacID)), startStop(startPin,true),running(runningPin,false,true,true),errStatus(errStatusPin,false,true,true)
{
	this->dacSetup();

}

Pump::~Pump() {
	// TODO Auto-generated destructor stub
}

bool Pump::setSpeed(int speed)
{
	if(speed>this->pumpFullSpeed)
		return false;

	this->pumpSpeed = speed;

	int data = speed;
	int command = 0x30;
	//data = data <<4;
	wiringPiI2CWriteReg8(this->dac,command,data);

	return true;
}

bool Pump::setSpeedPerc(double percentage)	//Percentage is a value between 0 and 1.  This covers the DAC range and does not incorporate the hardware offset and scaling
{
	if(percentage>1 || percentage<0)
		return false;
	this->pumpPerc = percentage;
	return this->setSpeed(percentage*this->pumpFullSpeed);
}

int Pump::getSpeed()
{
	return this->pumpSpeed;
}

double Pump::getSpeedPerc()
{
	return this->pumpPerc;
}
int Pump::getFullSpeed()
{

	return this->pumpFullSpeed;
}
bool Pump::setPumpOn(bool set)
{
	this->pumpOn = set;

	return this->startStop.setActive(set);
}

bool Pump::getPumpOn()
{
	return this->pumpOn;
}

bool Pump::statusUpdate()
{
	this->running.update();
	this->errStatus.update();

	return true;
}

bool Pump::getPumpRunning()
{
	return this->running.getState();
}

bool Pump::getPumpErrStatus()
{
	return this->errStatus.getState();
}

bool Pump::dacSetup()
{
	int command = 0x40;
	int data = 0x0800;

	wiringPiI2CWriteReg16(this->dac,command,data);
	data = 820;
	command = 0x30;
	data = data <<4;
	wiringPiI2CWriteReg8(this->dac,command,data);

	return true;
}
