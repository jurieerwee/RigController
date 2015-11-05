/*
 * Timers.cpp
 *
 *  Created on: 28 Sep 2015
 *      Author: Jurie
 */

#include "Timers.h"
#include <unistd.h>
#include <signal.h>
#include <iostream>
#include <stdio.h>

namespace timers {

bool delay1 = false;	//used for 1second delay in states.
int delay1_c = 1;

bool delay30 = false; //30second delay
int delay30_c =1;

bool sendUpdate = false;
int sendUpdate_c = 1;

bool controllerPulse = false;
int controllerPulse_c =1;

bool flowTimeout = false;
int flowTimeout_c = 1;

void timerHanlder(int in)
{
	signal(SIGALRM,SIG_IGN);

	if(delay1_c>0)
	{
		if(--delay1_c==0)
		//if(--delay1_c==0)
		{
			delay1 = true;
		}
	}

	if(delay30_c>0)
	{
		if(--delay30_c==0)
		{
			delay30 = true;
		}
	}

	//Repeats
	if(--sendUpdate_c==0)
	{
		sendUpdate_c = (int)(0.5*FREQ);
		sendUpdate = true;
	}

	//ControllerPulse is set with every trigger of interrupt
	controllerPulse = true;

	//Repeats
	if(--flowTimeout_c == 0)
	{
		flowTimeout_c = 60*FREQ;
		flowTimeout = true;
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
	int old = delay30_c;
	delay30_c = 30*FREQ;
	delay30 = false;
	return old;
}

int reset_delay1()	//delay1 now 2 seconds long.
{
	int old = delay1_c;
	delay1_c = 2*FREQ;
	delay1 = false;
	return old;
}

int reset_sendUpdate()
{
	int old = sendUpdate_c;
	sendUpdate_c = (int)(0.5*FREQ);
	sendUpdate = false;
	return old;
}

int reset_controllerPulse()
{
	int old = controllerPulse_c;
	controllerPulse_c = (int)(0.5*FREQ);
	controllerPulse = false;
	return old;
}

int reset_flowTimeout()
{
	int old = flowTimeout_c;
	flowTimeout_c = 60*FREQ;
	flowTimeout = false;
	return old;
}


} /* namespace timers */
