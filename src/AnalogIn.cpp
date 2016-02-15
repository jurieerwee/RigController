/*
 * AnalogIn.cpp
 *
 *  Created on: 21 Aug 2015
 *      Author: Jurie
 */

#include "AnalogIn.h"
#include <wiringPiI2C.h>

AnalogIn::AnalogIn(int adcID, int window) : adc(wiringPiI2CSetup(adcID)), windowLength(window)
{

}

AnalogIn::~AnalogIn()
{

}

int AnalogIn::sampleChannel(int channel)
{
	//TODO!! Hierdie moet verander.  Hierdie I2C command lees die vorige conversion en gee opdrag van watter kanaal om volgende te lees, so met multi channels gaan die verkeerd werk

	int address = 1 << channel;
	address = address << 4;

	int value =  wiringPiI2CReadReg16 (this->adc,address) ;
	value = (value>>8)+ (value<<8);	//Correct Endian
	if ((value & 0x8000) > 0)
			return -1;

	if (((value>>12) & 3) != channel)
	{
		this->data[channel] =-1;
		return false;
	}

	this->data[channel] =  ((value & 0x0FFF)>>2) & 0x03FF;
	//Assuming only doing pressure!  Change this when adding other analog.
	this->dataSet.push_front(this->data[channel]);
	this->dataSum += this->data[channel];
	this->dataSumSqrd += (this->data[channel]*this->data[channel]);
	this->setdataSum += this->data[channel];
	this->setdataCount +=1;

	if(this->windowLength < this->dataSet.size())
	{
		this->dataSum -= this->dataSet.back();
		this->dataSumSqrd -= (this->dataSet.back()*this->dataSet.back());
		this->dataSet.pop_back();
	}
	this->data[channel] = this->dataSum / this->dataSet.size();
	this->dataVar[channel] = 1.0* this->dataSumSqrd / this->dataSet.size() - (this->data[channel]*this->data[channel]);
	this->setdata[channel] = 1.0 * this->setdataSum / this->setdataCount;

	return true;
}

int AnalogIn::readChannel(int channel)
{
	return this->data[channel];
}

double AnalogIn::readChannelVar(int channel)
{
	return this->dataVar[channel];
}

double AnalogIn::readChannelVarScaled(int channel)
{
	int raw = this->readChannelVar(channel);

	return raw==-1?-1: raw * this->scale[channel]* this->scale[channel];
}

double AnalogIn::readChannelScaled(int channel)	//Returns the scaled value read from the ADC.
{
	int raw = this->readChannel(channel);
	//TODO: Consider better way of sending error, since -1 might be a valid value.

	return raw==-1?-1: raw * this->scale[channel] + this->offset[channel];

}

double AnalogIn::readSetChannel(int channel)
{
	return this->setdata[channel];
}

double AnalogIn::readSetChannelScaled(int channel)
{
	return this->readSetChannel(channel)==-1 ? -1 : this->readSetChannel(channel)*this->scale[channel] + this->offset[channel];
}

bool AnalogIn::setScale(int ch, double _offset, double _scale)
{
	this->offset[ch] = _offset;
	this->scale[ch] = _scale;

	return true;
}

bool AnalogIn::resetCounters(int channel)
{
	this->setdataCount = 0;
	this->setdataSum = 0;
	this->setdata[channel] = 0;
	return true;
}

bool AnalogIn::activateChannel(int channel)
{

}

bool AnalogIn::setup()
{
	int address = 0;
	int data =0;

	//setup
	address = 0x02;
	data = 0x48;
	int resp = wiringPiI2CWriteReg8(this->adc,address,data);


	data = 0xFFFF;
	address = 0x0C;
	wiringPiI2CWriteReg16 (this->adc,address,data);

	return true;

}
