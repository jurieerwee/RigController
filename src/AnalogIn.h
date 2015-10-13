/*
 * AnalogIn.h
 *
 *  Created on: 21 Aug 2015
 *      Author: Jurie
 */

//Controls the entire ADC IC



#ifndef ANALOGIN_H_
#define ANALOGIN_H_

class AnalogIn
{
public:
	AnalogIn(int adcID);
	virtual ~AnalogIn();
	int readChannel(int channel);
	double readChannelScaled(int channel);
	bool activateChannel(int channel);
	bool setup();
	bool setScale(int ch, double offset, double scale);

private:

	int adc;
	//These channel numbers correspond to the ADC and not the PCB numbering.
	double offset[4];	//The offset to add to the 4 channels
	double scale[4];	//The scale by which to adjust the 4 channels.
};

#endif /* ANALOGIN_H_ */

