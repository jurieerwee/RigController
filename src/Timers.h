/*
 * Timers.h
 *
 *  Created on: 28 Sep 2015
 *      Author: Jurie
 */

#ifndef SRC_TIMERS_H_
#define SRC_TIMERS_H_

#define FREQ 10	//Frequency of the signal in Hz

namespace timers {

	extern bool delay1;	//used for 1second delay in states.
	extern unsigned int delay1_c;

	extern  bool delay30; //30second delay
	extern unsigned in delay30_c;

	void timerHanlder(int in);
	void init();
	int reset_delay30();
	int reset_delay1();


} /* namespace timers */

#endif /* SRC_TIMERS_H_ */
