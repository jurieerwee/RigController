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

using namespace std;

deque<double> FlowInst::instRate;
double 	FlowInst::runningSum;
unsigned int		FlowInst::length;	//The number of elements taken into running average
struct 	timespec FlowInst::flowTime;
struct 	timespec FlowInst::flowTimePrev;
int		FlowInst::counter =0;
volatile bool FlowInst::dir;	//True = forward, false = backwards
int		FlowInst::dirPin;

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

	FlowInst::dir = !(bool)(digitalRead(this->dirPin));
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

double FlowMeter::getLastAve(int count)	//Calculates average of last 'count' pulses
{
	double sum = 0;
	piLock(0);	//Ensures instRate doenst change while calculating
	for(int i=0; i<count;i++)
	{
		sum+= FlowInst::instRate.at(i);
	}
	piUnlock(0);

	return sum / count * this->factor;
}

bool FlowMeter::clearCounter(void)
{
	FlowInst::counter = 0;
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



void FlowInst::flowInterrupt(void)
{
	FlowInst::flowTimePrev.tv_sec = FlowInst::flowTime.tv_sec;
	FlowInst::flowTimePrev.tv_nsec = FlowInst::flowTime.tv_nsec;
	clock_gettime(CLOCK_MONOTONIC,&FlowInst::flowTime);
	struct timespec flowDiff = diff(FlowInst::flowTimePrev,FlowInst::flowTime);
	double rate = 60e9/(flowDiff.tv_sec * 1e9 + flowDiff.tv_nsec);
	piLock(0);
	FlowInst::instRate.push_front(rate);
	FlowInst::runningSum += rate;
	if(FlowInst::instRate.size() > FlowInst::length)
	{
		FlowInst::runningSum -= FlowInst::instRate.back();
		FlowInst::instRate.pop_back();
	}
	piUnlock(0);
	counter++;
}

void FlowInst::dirInterrupt(void)
{
	FlowInst::dir = !(bool)digitalRead(FlowInst::dirPin);
	//cout << "Rise\n";
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
