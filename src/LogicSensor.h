/*
 * LogicSensor.h
 *
 *  Created on: 07 Jul 2015
 *      Author: Jurie
 */

#ifndef LOGICSENSOR_H_
#define LOGICSENSOR_H_

class LogicSensor {
public:
	LogicSensor(int _pin, bool _pull, bool _pullUp, bool _nopen);

	virtual ~LogicSensor();
	bool update();
	bool getState();
	bool getHigh();

private:
	const int pin;
	const bool pull;
	const bool pullUp = true;
	bool isHigh = false;
	bool nopen = true; //Normally open sensor or not.

};

#endif /* LOGICSENSOR_H_ */
