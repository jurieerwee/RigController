/*
 * AnalogIn.h
 *
 *  Created on: 21 Aug 2015
 *      Author: Jurie
 */

//Controls the entire ADC IC



#ifndef ANALOGIN_H_
#define ANALOGIN_H_


#include <deque>
using namespace std;

class AnalogIn
{
public:
	AnalogIn(int adcID, int window);
	virtual ~AnalogIn();
	int readChannel(int channel);
	double readChannelVar(int channel);
	double readChannelVarScaled(int channel);
	double readChannelScaled(int channel);
	bool activateChannel(int channel);
	bool setup();
	bool setScale(int ch, double offset, double scale);
	int sampleChannel(int channel);

private:

	int adc;
	//These channel numbers correspond to the ADC and not the PCB numbering.
	double offset[4];	//The offset to add to the 4 channels
	double scale[4];	//The scale by which to adjust the 4 channels.
	//Stored data when read,sothat it can be fetched without re-reading.  Enables synchronous reading.
	int data[4] ={-1,-1,-1,-1};
	double dataVar[4] = {-1,-1,-1,-1};
	deque<int> dataSet;
	int dataSum =0;
	int dataSumSqrd = 0;
	const int windowLength;
};

#endif /* ANALOGIN_H_ */

