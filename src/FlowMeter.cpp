/*
 * FlowMeter.cpp
 *
 *  Created on: 21 Sep 2015
 *      Author: Jurie
 */

#include "FlowMeter.h"
#include <wiringPi.h>
#include <time.h>
#include <deque>
#include <stdio.h>
#include <iostream>
#include "Timers.h"

using namespace std;

deque<double> FlowInst::instRate;
double 	FlowInst::runningSum;
unsigned int		FlowInst::length;	//The number of elements taken into running average
struct 	timespec FlowInst::flowTime;
struct 	timespec FlowInst::flowTimePrev;
int		FlowInst::counter =0;
double	FlowInst::setSum =0;
volatile bool FlowInst::dir;	//True = forward, false = backwards
int		FlowInst::dirPin;
bool FlowInst::firstPulse = true;  //Used to indicate that there is no previous pulse

FlowMeter::FlowMeter(int _flowPin, int _dirPin, bool _pull, bool _pullUp,  double _factor, int runLength): flowPin(_flowPin), dirPin(_dirPin), pull(_pull),pullUp(_pullUp), factor(_factor)
{
	this->init();
	this->setLength(runLength);

}

FlowMeter::~FlowMeter() {
	// TODO Auto-generated destructor stub
}


void FlowMeter::init()
{
	if(this->flowPin>-1)
	{
		pinMode(this->flowPin, INPUT);
		pullUpDnControl(this->flowPin, this->pull ?(this->pullUp?PUD_UP:PUD_DOWN):PUD_OFF);
		if(wiringPiISR (this->flowPin,INT_EDGE_FALLING, &FlowInst::flowInterrupt)<0)
		{
			//printf( "Unable to setup ISR\n");
			//TODO, gooi error of iets.
		}
	}

	FlowInst::dirPin = this->dirPin;
	if(this->dirPin>-1)
	{
		pinMode(this->dirPin, INPUT);
		pullUpDnControl(this->dirPin, this->pull ?(this->pullUp?PUD_UP:PUD_DOWN):PUD_OFF);
		if(wiringPiISR (this->dirPin,INT_EDGE_BOTH, &FlowInst::dirInterrupt)<0)
		{
			printf( "Unable to setup ISR\n");
			//TODO, gooi error of iets.
		}
	}

	FlowInst::dir = (bool)(digitalRead(this->dirPin));
}

bool FlowMeter::getDir(void)
{
	return FlowInst::dir;
}

double FlowMeter::getRunningAve(void)
{
	double ave =0;
	piLock(0);
	ave = FlowInst::runningSum / FlowInst::instRate.size() * this->factor;\
	piUnlock(0);

	if(FlowInst::instRate.size()==0)
		return 0;

	return ave;
}

double FlowMeter::getSetAve()	//Calculates average since reset.  Note:  Zero pulse not in effect for this average.
{
	double rate = 0;

	if(FlowInst::counter<2)
		return 0;

	piLock(0);	//Ensures instRate doenst change while calculating
	rate = FlowInst::setSum / (FlowInst::counter-1); //Minus one since first pulse was initiating trigger and not added.
	piUnlock(0);

	return rate * this->factor;
}

bool FlowMeter::clearCounter(void)
{
	FlowInst::counter = 0;
	FlowInst::setSum = 0;
	return true;
}

int FlowMeter::getCounter(void)
{
	return FlowInst::counter;
}

int FlowMeter::setLength(int _length)	//Returns current length
{
	int temp = FlowInst::length;
	FlowInst::length = _length;
	return temp;
}

void FlowMeter::zeroPulse()	//This method is to be called after a timeout is triggered in order to add flow rate of zero, since no interrupts will be trigger at zero flow
{
	piLock(0);
	FlowInst::instRate.push_front(0);
	if(FlowInst::instRate.size() > FlowInst::length)
	{
		FlowInst::runningSum -= FlowInst::instRate.back();
		FlowInst::instRate.pop_back();
	}
	piUnlock(0);
}



void FlowInst::flowInterrupt(void)
{
	if(FlowInst::firstPulse)
	{
		clock_gettime(CLOCK_MONOTONIC,&FlowInst::flowTime);
		//Reset timeout
		timers::reset_flowTimeout();
		FlowInst::firstPulse = false;
	}
	else
	{
		FlowInst::flowTimePrev.tv_sec = FlowInst::flowTime.tv_sec;
		FlowInst::flowTimePrev.tv_nsec = FlowInst::flowTime.tv_nsec;
		clock_gettime(CLOCK_MONOTONIC,&FlowInst::flowTime);
		//Reset timeout
		timers::reset_flowTimeout();
		struct timespec flowDiff = diff(FlowInst::flowTimePrev,FlowInst::flowTime);
		double rate = 60e9/(flowDiff.tv_sec * 1e9 + flowDiff.tv_nsec);
		piLock(0);
		FlowInst::instRate.push_front(rate);
		FlowInst::runningSum += rate;
		FlowInst::setSum += FlowInst::counter>0?rate:0;
		if(FlowInst::instRate.size() > FlowInst::length)
		{
			FlowInst::runningSum -= FlowInst::instRate.back();
			FlowInst::instRate.pop_back();
		}
		FlowInst::counter++;
		piUnlock(0);
	}
	digitalWrite(21,!(bool)digitalRead(21));
}

void FlowInst::dirInterrupt(void)
{
	FlowInst::dir = (bool)digitalRead(FlowInst::dirPin);
	//Reset flow measurement on direction change
	FlowInst::firstPulse = true;
	piLock(0);
	FlowInst::instRate.clear();
	FlowInst::instRate.push_front(0);
	FlowInst::runningSum =0;
	piUnlock(0);


}



struct timespec FlowInst::diff(struct timespec start, struct timespec end)
{
	struct timespec temp;
	if ((end.tv_nsec-start.tv_nsec)<0) {
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1e9+end.tv_nsec-start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp;
}
