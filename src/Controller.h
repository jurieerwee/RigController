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

namespace po = boost::program_options;

#include "Rig.h"

class Controller {
public:
	Controller();
	virtual ~Controller();


	int loop();



private:
	po::variables_map vm;
	Rig rig;

	enum State {WAIT,PRIME1,PRIME2,PRIME3,FILL,FORCEFILL,PUMPING,PRESSURE_TRANS,PRESSURE_HOLD,OVERRIDE,ERROR};
	State state = WAIT;
	enum TankState {TRANSIENT=0,EMPTY,FULL,TANK_ERROR};

	double pressThreash;	//Pressure threshold where system is considered pressurised

	inline int initWait();
	inline int initPrime1();
	inline int initPrime2();
	inline int initPrime3();
	inline int initFill();
	inline int initForceFill();
	inline int initPumping();
	inline int initPressureTrans();
	inline int initPressureHold();
	inline int initOverride();
	inline int initError();

	/*inline int loopWait();
	inline int loopPrime1();
	inline int loopPrime2();
	inline int loopPrime3();
	inline int loopFill();
	inline int loopForceFill();
	inline int loopPumping();
	inline int loopPressureTrans();
	inline int loopPressureHold();
	inline int loopOverride();
	inline int loopError();*/

	inline bool isPressure();	//Check whether pressure is high enough
	inline 	TankState getTank();	//Translate two tank sensors to a state
	inline bool getReverseFlow()
};

#endif /* SRC_CONTROLLER_H_ */
