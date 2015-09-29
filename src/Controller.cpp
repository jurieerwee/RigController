/*
 * Controller.cpp
 *
 *  Created on: 28 Sep 2015
 *      Author: Jurie
 */

#include "Controller.h"
#include "Rig.h"
#include "Timers.h"

#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

Controller::Controller() : rig((this->vm)) {
	// TODO Auto-generated constructor stub

}

Controller::~Controller() {
	// TODO Auto-generated destructor stub
}

inline int Controller::loopWait()
{

}
inline int Controller::loopPrime1()
{
	if(this->getTank()==TANK_ERROR)
	{
		this->initError();
		return 0;
	}
	else if(!timers::delay1)
	{
		return 1;
	}
	else if(!this->isReverseFlow())
	{
		this->initError();
		return 0;
	}
	else
	{
		this->initPrime2();
	}

	return 1;

}

inline int Controller::loopPrime2()
{
	if(this->getTank()==TANK_ERROR)
	{
		this->initError();
		return 0;
	}
	else if(!this->isReverseFlow())
	{
		this->initError();
		return 0;
	}
	else if(!timers::delay1)
	{
		return 1;
	}

	else
	{
		this->initPrime3();
	}
	return 1;
}

inline int Controller::loopPrime3()
{
	if(this->getTank()==TANK_ERROR)
	{
		this->initError();
		return 0;
	}
	else if(!this->isReverseFlow())
	{
		this->initError();
		return 0;
	}
	//TODO:  insert pre-empt code for when wait is called earlier.
	else if(this->getTank()==FULL)
	{
		this->initWait();
	}

	return 1;
}
inline int Controller::loopFill()
{
	if(this->getTank()==TANK_ERROR)
	{
		this->initError();
		return 0;
	}
	else if(!timers::delay1)
	{
		return 1;
	}
	else if(!this->isReverseFlow())
	{
		this->initError();
		return 0;
	}
	else if(this->getTank()==FULL)
	{
		this->initWait();
	}

	return 1;
}

inline int Controller::loopForceFill()
{
	return 1;
}

inline int Controller::loopPumping()
{
	if(this->getTank()==TANK_ERROR)
	{
		this->initError();
		return 0;
	}
	else if(this->rig.getPumpErrStatus())
	{
		this->initError();
		return 0;
	}
	else if(this->getTank()==EMPTY)
	{
		this->initWait();
	}
	else if(!timers::delay1)
	{
		return 1;
	}
	else if(!this->rig.getPumpRunning())
	{
		this->initError();
		return 0;
	}
	else if(!this->isPressure() && !this->isFlow())
	{
		this->initError();
		return 0;
	}

	return 1;
}

inline int Controller::loopPressureTrans()
{
	if(this->getTank()==TANK_ERROR)
	{
		this->initError();
		return 0;
	}
	else if(this->rig.getPumpErrStatus())
	{
		this->initError();
		return 0;
	}
	else if(this->getTank()==EMPTY)
	{
		this->initWait();
	}
	else if(!this->rig.getPumpRunning())
	{
		this->initError();
		return 0;
	}
	else if(!this->isPressure() && !this->isFlow())
	{
		this->initError();
		return 0;
	}

	//TODO:  Closed loop control

	return 1;
}

inline int Controller::loopPressureHold()
{
	if(this->getTank()==TANK_ERROR)
	{
		this->initError();
		return 0;
	}
	else if(this->rig.getPumpErrStatus())
	{
		this->initError();
		return 0;
	}
	else if(this->getTank()==EMPTY)
	{
		this->initWait();
	}
	else if(!this->rig.getPumpRunning())
	{
		this->initError();
		return 0;
	}
	else if(!this->isPressure() && !this->isFlow())
	{
		this->initError();
		return 0;
	}
	return 1;
}
inline int Controller::loopOverride();
inline int Controller::loopError();

inline int Controller::initWait()
{
	//Precondition check
	if(!(this->state == WAIT || this->state == PRIME3 || this->state == FILL || this->state == FORCEFILL || this->state == PUMPING || this->state == PRESSURE_HOLD || this->state == OVERRIDE))
	{
		return 0;	//Cannot enter
	}

	this->state = WAIT;

	//SET STATE
	bool success = true;
	success &= this->rig.stopPumpOnly();
	success &= this->rig.closeInflowValveOnly();
	success &= this->rig.closeOutflowValveOnly();
	success &= this->rig.closeReleaseValveOnly();

	if(!success)
	{
		this->initError();
		return -1;
	}
	return 1;
}


inline int Controller::initPrime1()
{
	//Precondition check
	if(!this->isPressure() || !(this->state == WAIT || this->state == PRIME1))
	{
		return 0;	//Cannot enter
	}

	this->state = PRIME1;

	//SET STATE
	bool success = true;
	success &= this->rig.stopPumpOnly();
	success &= this->rig.openInflowValveOnly();
	success &= this->rig.closeOutflowValveOnly();
	success &= this->rig.closeReleaseValveOnly();

	timers::reset_delay1();	//Start 1 second delay

	if(!success)
	{
		this->initError();
		return -1;	//Error on enter
	}
	return 1;
}

inline int Controller::initPrime2()
{
	//Precondition check
	if(!(this->state == PRIME2 || this->state == PRIME1))
	{
		return 0;	//Cannot enter
	}

	this->state = PRIME2;

	//SET STATE
	bool success = true;
	success &= this->rig.stopPumpOnly();
	success &= this->rig.openInflowValveOnly();
	success &= this->rig.openOutflowValveOnly();
	success &= this->rig.closeReleaseValveOnly();

	timers::reset_delay1();	//Start 1 second delay

	if(!success)
	{
		this->initError();
		return -1;	//Error on enter
	}
	return 1;
}
inline int Controller::initPrime3()
{
	//Precondition check
	if( !(this->state == PRIME2 || this->state == PRIME3))
	{
		return 0;	//Cannot enter
	}

	this->state = PRIME3;
	//SET STATE
	bool success = true;
	success &= this->rig.stopPumpOnly();
	success &= this->rig.openInflowValveOnly();
	success &= this->rig.openOutflowValveOnly();
	success &= this->rig.openReleaseValveOnly();

	timers::reset_delay30();	//Start 30 second delay

	if(!success)
	{
		this->initError();
		return -1;	//Error on enter
	}
	return 1;
}

inline int Controller::initFill()
{
	//Precondition check
	if(!this->isPressure() || !(this->state == FILL || this->state == WAIT))
	{
		return 0;	//Cannot enter
	}

	this->state = FILL;

	//SET STATE
	bool success = true;
	success &= this->rig.stopPumpOnly();
	success &= this->rig.openInflowValveOnly();
	success &= this->rig.closeOutflowValveOnly();
	success &= this->rig.closeReleaseValveOnly();

	timers::reset_delay1();	//Start 1 second delay

	if(!success)
	{
		this->initError();
		return -1;	//Error on enter
	}
	return 1;
}

inline int Controller::initForceFill()
{
	//Precondition check
	if( !(this->state == FORCEFILL || this->state == WAIT))
	{
		return 0;	//Cannot enter
	}
	this->state = FORCEFILL;
		//SET STATE
	bool success = true;
	success &= this->rig.stopPumpOnly();
	success &= this->rig.openInflowValveOnly();
	success &= this->rig.closeOutflowValveOnly();
	success &= this->rig.closeReleaseValveOnly();
		if(!success)
	{
		this->initError();
		return -1;	//Error on enter
	}
	return 1;
}

inline int Controller::initPumping()
{
	//Precondition check
	if(this->getTank()==EMPTY || !(this->state == PUMPING || this->state == WAIT || this->state == PRESSURE_HOLD))
	{
		return 0;	//Cannot enter
	}

	this->state = PUMPING;

	//SET STATE
	bool success = true;
	success &= this->rig.startPumpOnly();
	success &= this->rig.closeInflowValveOnly();
	success &= this->rig.openOutflowValveOnly();
	success &= this->rig.closeReleaseValveOnly();

	timers::reset_delay1();	//Start 1 second delay

	if(!success)
	{
		this->initError();
		return -1;	//Error on enter
	}
	return 1;
}

inline int Controller::initPressureTrans()
{
	//Precondition check
	if(!(this->state == PUMPING || this->state == PRESSURE_HOLD || this->state == PRESSURE_TRANS))
	{
		return 0;	//Cannot enter
	}

	this->state = PRESSURE_TRANS;

	//SET STATE
	bool success = true;
	success &= this->rig.startPumpOnly();
	success &= this->rig.closeInflowValveOnly();
	success &= this->rig.openOutflowValveOnly();
	success &= this->rig.closeReleaseValveOnly();

	if(!success)
	{
		this->initError();
		return -1;	//Error on enter
	}
	return 1;
}

inline int Controller::initPressureHold()
{
	//Precondition check
	if(!(this->state == PRESSURE_HOLD || this->state == PRESSURE_TRANS))
	{
		return 0;	//Cannot enter
	}

	this->state = PRESSURE_HOLD;

	//SET STATE
	bool success = true;
	success &= this->rig.startPumpOnly();
	success &= this->rig.closeInflowValveOnly();
	success &= this->rig.openOutflowValveOnly();
	success &= this->rig.closeReleaseValveOnly();

	if(!success)
	{
		this->initError();
		return -1;	//Error on enter
	}
	return 1;
}

inline int Controller::initOverride()
{
	//Precondition check
	if(!(this->state == OVERRIDE || this->state == WAIT))
	{
		return 0;	//Cannot enter
	}

	this->state = OVERRIDE;

	//SET STATE
	bool success = true;
	success &= this->rig.stopPumpOnly();
	success &= this->rig.closeInflowValveOnly();
	success &= this->rig.closeOutflowValveOnly();
	success &= this->rig.closeReleaseValveOnly();

	if(!success)
	{
		this->initError();
		return -1;	//Error on enter
	}
	return 1;
}

inline int Controller::initError()
{
	//Precondition check
	//NONE

	this->state = ERROR;

	//SET STATE
	bool success = true;
	success &= this->rig.stopPumpOnly();
	success &= this->rig.closeInflowValveOnly();
	success &= this->rig.closeOutflowValveOnly();
	success &= this->rig.closeReleaseValveOnly();

	if(!success)
	{
		return -1;	//Error on enter
	}
	return 1;
}

inline bool Controller::isPressure()	//Check whether pressure is high enough
{
	return this->rig.getSensor_Pressure() >= this->pressThreash;
}

inline Controller::TankState Controller::getTank()	//Translate two tank sensors to a state
{
	return (Controller::TankState)( (int)this->rig.getSensor_EmptyTank() + (int)(this->rig.getSensor_FullTank()<<1));	//00= transient, 01 = EMPTY, 10= FULL, 11 = ERROR

}

inline bool Controller::isReverseFlow()
{
	//TODO:  Define reverse flow

}

inline bool Controller::isFlow()
{

}

