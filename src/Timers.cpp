/*
 * Timers.cpp
 *
 *  Created on: 28 Sep 2015
 *      Author: Jurie
 */

#include "Timers.h"
#include <unistd.h>
#include <signal.h>

namespace timers {

void timerHanlder(int in)
{
	signal(SIGALRM,SIG_IGN);
	if(--delay1_c==0)
	{
		delay1_c = FREQ;
		delay1 = true;
	}

	if(--delay30_c==0)
	{
		delay30_c = 30*FREQ;
		delay30 = true;
	}
	signal(SIGALRM,timerHanlder);
}

void init()
{
	signal(SIGALRM,timerHanlder);
	ualarm(1e6/FREQ,1e6/FREQ);
}

int reset_delay30()
{
	int old = delay30;
	delay30 = 30*FREQ;
	delay30 = false;
	return old;
}

int reset_delay1()
{
	int old = delay1;
	delay1 = 1*FREQ;
	delay1 = false;
	return old;
}

} /* namespace timers */
