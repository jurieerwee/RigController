/*
 * LogicOut.h
 *
 *  Created on: 07 Jul 2015
 *      Author: Jurie
 */

#ifndef SRC_LOGICOUT_H_
#define SRC_LOGICOUT_H_

class LogicOut {
public:
	LogicOut(int _pin, bool _activeHigh);
	virtual ~LogicOut();

	bool setActive(bool act);
	bool getState();

private:
	const int pin;
	const bool activeHigh;
	bool activeState = false;

};

#endif /* SRC_LOGICOUT_H_ */
