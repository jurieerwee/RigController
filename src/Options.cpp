/*
 * Options.cpp
 *
 *  Created on: 15 Oct 2015
 *      Author: Jurie
 */

#include "Options.h"
#include "Logging.h"

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

namespace po = boost::program_options;
using namespace std;

Options::Options(): lg(my_logger::get()){
	// TODO Auto-generated constructor stub

}

Options::~Options() {
	// TODO Auto-generated destructor stub
}

bool Options::initOptions()
{
	po::options_description desc("System parameters");
	desc.add_options()
			("pumpFullSpeed",po::value<int>()->default_value(3700),"The pumps full speed in rpm")
			("pumpFullPressure",po::value<double>(),"The pump's 'pressure assosiated with full speed")
			("settleTime",po::value<int>()->default_value(60),"Leakage test settle time in seconds")
			("pressureMeasureInterval",po::value<int>()->default_value(30),"Time in seconds between pressure measurements")
			("pressureMeasureCount",po::value<int>()->default_value(6),"Number of pressure measurements to sample")
			("testPressure",po::value<vector<double>>()->composing(),"Test points for leakage test (pressures)")
			("tankFullPin",po::value<int>(),"WiringPi pin connected to tankFull sensor")
			("tankFullNO",po::value<int>()->default_value(1),"Set to 0 if N.C. sensor, else 0")
			("tankEmptyPin",po::value<int>(),"WiringPi pin connected to tankEmpty sensor")
			("tankEmptyNO",po::value<int>()->default_value(1),"Set to 0 if N.C. sensor, else 0")
			("inflowValvePin",po::value<int>(),"WiringPi pin connected to inflow valve")
			("inflowValveActv",po::value<int>()->default_value(1),"Set 1 if activated is HIGH, 0 if activated is LOW")
			("outflowValvePin",po::value<int>(),"WiringPi pin connected to outflow valve")
			("outflowValveActv",po::value<int>()->default_value(1),"Set 1 if activated is HIGH, 0 if activated is LOW")
			("isolateDuration",po::value<int>()->default_value(30),"Duration of the pipe isolation test")
			("isolateTolerance",po::value<int>()->default_value(1),"Minimum amount of water that must flow for pipe to be considered not-isolated")
			("dacID",po::value<int>(),"DAC's I2C address in decimal")
			("startPin",po::value<int>(),"Pump start/stop pin number")
			("runningPin",po::value<int>(),"Pin number of pump running status indicator")
			("errStatusPin",po::value<int>(),"Pin number of pump error status indicator")
			("adcID",po::value<int>(),"IDC's I2C address in decimal")
			("pressureCh",po::value<int>(),"ADC channel pressure sensor is connected to")
			("pressureOffset",po::value<double>(),"Offset in the scaling of pressure measurement")
			("pressureScale",po::value<double>(),"Scale factor in the scaling of pressure measurement")
			("pressureThreshold",po::value<double>(),"Pressure at wich the system is considered pressurised")
			("flow1Pin",po::value<int>(),"Pin number of flow1")
			("flow1dirPin",po::value<int>(),"Pin number of flow1's direction indicator")
			("flow1Pull",po::value<bool>(),"Set 1 if internal level pull is required")
			("flow1PullUp",po::value<bool>()->default_value(1),"Set 1 if pull up, 0 if pull down")
			("flow1Factor",po::value<double>(),"Flow1's multiplication factor of pulse rate to flow rate")
			("flow1RunLength",po::value<int>(),"Flow1's number of pulses taken for running average")
			("releaseValvePin",po::value<int>(),"WiringPi pin connected to releaseValve")
			("releaseValveActv",po::value<int>()->default_value(1),"Set 1 if activated is HIGH, 0 if activated is LOW")
			("emergencyBtnPin",po::value<int>(),"WiringPi pin connected to emergency button")
			("emergencyBtnPull", po::value<bool>(),"Set 1 if emergency button level must be pull internally, else 0")
			("emergencyBtnPullUp", po::value<bool>(),"Set 1 if emergency button level must be pull high, 0 if pulled low.")
			("emergencyBtnNO", po::value<bool>(),"Set to 0 if N.C. sensor, else 0")
			("pressSettledTolerance",po::value<double>(),"Pressure tolerance whereby system considered as stable (PI controller)")
			("pressSettledCount",po::value<int>(), "Number of consecutive samples that must be within threshold to be considered stable(PI controller")
			("ki",po::value<double>(),"Intergral factor for pressure controller")
			("kp",po::value<double>(),"Proportional factor for pressure controller")
			;
	ifstream ifs("config.cfg");
	if(!ifs)
	{
		BOOST_LOG_SEV(this->lg,logging::trivial::error) << "Configuration file not loaded. Exiting";
		exit(1);
	}

	po::store(po::parse_config_file(ifs,desc),this->vm);
	po::notify(this->vm);

	//Pin check- checking that no device is set to the same pin
	if(this->vm["tankFullPin"].as<int>()==this->vm["tankEmptyPin"].as<int>() ||this->vm["tankFullPin"].as<int>()==this->vm["outflowValvePin"].as<int>() ||this->vm["tankFullPin"].as<int>()==this->vm["inflowValvePin"].as<int>()\
	  ||this->vm["tankEmptyPin"].as<int>()==this->vm["outflowValvePin"].as<int>() ||this->vm["tankEmptyPin"].as<int>()==this->vm["inflowValvePin"].as<int>()\
	  ||this->vm["outflowValvePin"].as<int>()==this->vm["inflowValvePin"].as<int>())
	//TODO: change this! ^^^ Not all pin are checked
	{
		BOOST_LOG_SEV(this->lg,logging::trivial::error) << "WiringPi pin assignment conflict";
		exit(1);
	}

	return true;
}

po::variables_map& Options::getVM()
{
	return this->vm;
}
