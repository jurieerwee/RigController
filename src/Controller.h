/*
 * Controller.h
 *
 *  Created on: 28 Sep 2015
 *      Author: Jurie
 */

#ifndef SRC_CONTROLLER_H_
#define SRC_CONTROLLER_H_

#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/log/trivial.hpp>

#include <iostream>
#include <fstream>

using namespace std;

namespace po = boost::program_options;
namespace src = boost::log::sources;

#include "Rig.h"
//#include "MessageInterpreter.h"

void *ctrlThread();

class Controller {
public:
	Controller(po::variables_map& vm_);
	virtual ~Controller();

	enum State {IDLE,IDLE_PRES,PRIME1,PRIME2,PRIME3, PRIME4,FILL,FORCEFILL,PUMPING,PRESSURE_TRANS,PRESSURE_HOLD,OVERRIDE,ERROR};
	int loop();

	map<Controller::State,string> stateString;

private:
	friend class MessageInterpreter;

	//po::variables_map vm;
	src::severity_logger_mt<>& lg;
	Rig rig;

	State state = IDLE;
	enum TankState {TRANSIENT=0,EMPTY,FULL,TANK_ERROR};

	const double presThresh;	//Pressure threshold where system is considered pressurised
	double setPressure =0;		//Target pressure
	double setPercentage =0;	//Desired pump percentage
	const double pressSettledTolerance;	//Tolerance from desired pressure during pressure transient
	const int pressSettledCount;	//Number of samples that must consecutively be within pressSettledTolerance to be considered settled.
	int pressSettledCounter = 0;
	const double ki;
	const double kp;
	const double h_on;
	const double h_off;
	double ui;

	ofstream dataDumpFile;
	bool dump = false;

	int changeState(State newState, bool cmd);
	int setDesiredPumpPerc(double in);

	inline int initIdle();
	inline int initIdlePres();
	inline int initPrime1();
	inline int initPrime2();
	inline int initPrime3();
	inline int initPrime4();
	inline int initFill();
	inline int initForceFill();
	inline int initPumping();
	inline int initPressureTrans();
	inline int initPressureHold();
	inline int initOverride();
	inline int initError();

	inline int loopIdle();
	inline int loopIdlePres();
	inline int loopPrime1();
	inline int loopPrime2();
	inline int loopPrime3();
	inline int loopPrime4();
	inline int loopFill();
	inline int loopForceFill();
	inline int loopPumping();
	inline int loopPressureTrans();
	inline int loopPressureHold();
	inline int loopOverride();
	inline int loopError();

	inline bool piControl();

	inline bool isPressure();	//Check whether pressure is high enough
	inline 	TankState getTank();	//Translate two tank sensors to a state
	inline bool isReverseFlow();
	inline bool isForwardFlow();

	bool initDataDump();
	inline bool dataDump();
	bool stopDataDump();
};

#endif /* SRC_CONTROLLER_H_ */
