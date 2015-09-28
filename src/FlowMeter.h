/*
 * FlowMeter.h
 *
 *  Created on: 21 Sep 2015
 *      Author: Jurie
 */

#ifndef SRC_FLOWMETER_H_
#define SRC_FLOWMETER_H_

#include <vector>
#include <stdlib.h>
#include <unistd.h>
#include <deque>
#include <time.h>

using namespace std;

namespace FlowInst
{
	extern deque<double> instRate;
	extern double 	runningSum;
	extern unsigned int		length;	//The number of elements taken into running average
	extern struct 	timespec flowTime;
	extern struct 	timespec flowTimePrev;
	extern int		counter;
	extern volatile bool dir;	//True = forward, false = backwards

	void flowInterrupt(void);
	void dirInterruptRise(void);
	void dirInterruptFall(void);
	struct timespec diff(struct timespec start, struct timespec end);

}

class FlowMeter {
public:
	FlowMeter(int _flowPin, int _dirPin, bool _pull, bool _pullUp,  double _factor, int runLength);
	virtual ~FlowMeter();
	void init(void);
	bool getDir(void);
	double getRunningAve(void);
	bool clearCounter(void);
	int getCounter(void);
	double getLastAve(int count);
	int setLength(int _length);	//Returns current length

private:
	const int flowPin;
	const int dirPin;
	const bool pull;
	const bool pullUp;
	const int factor;
};

#endif /* SRC_FLOWMETER_H_ */
