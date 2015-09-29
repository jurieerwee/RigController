/*
 * Pump.h
 *
 *  Created on: 07 Jul 2015
 *      Author: Jurie
 */

#ifndef SRC_PUMP_H_
#define SRC_PUMP_H_

#include "LogicOut.h"
#include "LogicSensor.h"

class Pump {
public:
	Pump(int _fullSpeed, int _dacID, int startPin, int runningPin, int errStatusPin);
	virtual ~Pump();

	bool setSpeed(int speed);
	bool setSpeedPerc(double percentage);
	int getSpeed();
	int getFullSpeed();
	bool setPumpOn(bool set);
	bool getPumpOn();
	bool statusUpdate();
	bool getPumpRunning();
	bool getPumpErrStatus();

private:
	//Setup
	int pumpFullSpeed;
	//State
	bool pumpOn = false;
	int pumpSpeed;

	const int dacID; //I2C ID for DAC IC
	int dac;	//Handle to DAC
	LogicOut startStop;
	LogicSensor running;
	LogicSensor errStatus;

	bool dacSetup();


};

#endif /* SRC_PUMP_H_ */
