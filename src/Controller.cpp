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

int Controller::changeState(State newState, bool cmd)
{
	switch(newState)
	{
	case ERROR:
		return initError();
	case IDLE:
		if ((this->state == IDLE_PRES || this->state == FILL || this->state == PRIME4 || this->state == PUMPING ||\
				this->state == PRESSURE_TRANS || this->state == PRESSURE_HOLD ||this->state == OVERRIDE ||this->state == FORCEFILL) || \
				(this->state == PRIME3 && !cmd) || (this->state==ERROR && !this->rig.getEmerBtn() && cmd))
		{
			if(!this->initIdle())
			{
				this->changeState(ERROR);
				return -1;
			}
			return true;
		}
		else
			return false;
		break;

	case IDLE_PRES:
		if (this->state == IDLE && !cmd)
		{
			if(!this->initIdlePres())
			{
				this->changeState(ERROR);
				return -1;
			}
			return true;
		}
		else
			return false;
		break;
	case PRIME1:
		if (this->state == IDLE_PRES && cmd)
		{
			if(!this->initPrime1())
			{
				this->changeState(ERROR);
				return -1;
			}
			return true;
		}
		else
			return false;
		break;
	case PRIME2:
		if (this->state == PRIME1 && !cmd)
		{
			if(!this->initPrime2())
			{
				this->changeState(ERROR);
				return -1;
			}
			return true;
		}
		else
			return false;
		break;
	case PRIME3:
		if (this->state == PRIME2 && !cmd)
		{
			if(!this->initPrime3())
			{
				this->changeState(ERROR);
				return -1;
			}
			return true;
		}
		else
			return false;
		break;
	case PRIME4:
		if (this->state == PRIME4 && !cmd)
		{
			if(!this->initPrime4())
			{
				this->changeState(ERROR);
				return -1;
			}
			return true;
		}
		else
			return false;
		break;
	case FILL:
		if (this->state == IDLE_PRES && cmd)
		{
			if(!this->initFill())
			{
				this->changeState(ERROR);
				return -1;
			}
			return true;
		}
		else
			return false;
		break;
	case FORCEFILL:
		if ((this->state == IDLE_PRES || this->state ==IDLE) && cmd)
		{
			if(!this->initForceFill())
			{
				this->changeState(ERROR);
				return -1;
			}
			return true;
		}
		else
			return false;
		break;
	case PUMPING:
		if ((this->state == IDLE_PRES || this->IDLE || this->PRESSURE_HOLD) && cmd)
		{
			if(!this->initPumping())
			{
				this->changeState(ERROR);
				return -1;
			}
			return true;
		}
		else
			return false;
		break;
	case PRESSURE_TRANS:
		if ((this->state == PUMPING || this->PRESSURE_HOLD) && cmd)
		{
			if(!this->initPressureTrans())
			{
				this->changeState(ERROR);
				return -1;
			}
			return true;
		}
		else
			return false;
		break;
	case PRESSURE_HOLD:
		if ((this->PRESSURE_TRANS) &&!cmd)
		{
			if(!this->initPumping())
			{
				this->changeState(ERROR);
				return -1;
			}
			return true;
		}
		else
			return false;
		break;
	case OVERRIDE:
		if ((this->state == IDLE_PRES || this->IDLE) && cmd)
		{
			if(!this->initPumping())
			{
				this->changeState(ERROR);
				return -1;
			}
			return true;
		}
		else
			return false;
		break;
	default:
		return false;

	}

	return false;
}

inline int Controller::loopIdle()
{

}

inline int Controller::loopIdlePres()
{

}

inline int Controller::loopPrime1()
{
	if(this->getTank()==TANK_ERROR)
	{
		this->initError();
		return false;
	}
	else if(!timers::delay1)
	{
		return true;
	}
	else if(!this->isReverseFlow())
	{
		this->initError();
		return false;
	}
	else
	{
		this->initPrime2();
	}

	return true;

}

inline int Controller::loopPrime2()
{
	if(this->getTank()==TANK_ERROR)
	{
		this->initError();
		return false;
	}
	else if(!this->isReverseFlow())
	{
		this->initError();
		return false;
	}
	else if(!timers::delay1)
	{
		return true;
	}

	else
	{
		this->initPrime3();
	}
	return true;
}

inline int Controller::loopPrime3()
{
	if(this->getTank()==TANK_ERROR)
	{
		this->initError();
		return false;
	}
	else if(!this->isReverseFlow())
	{
		this->initError();
		return false;
	}
	//TODO:  insert pre-empt code for when wait is called earlier.
	else if(this->getTank()==FULL)
	{
		this->initIdle();
	}

	return true;
}

inline int Controller::loopPrime4()
{
	if(this->getTank()==TANK_ERROR)
	{
		this->initError();
		return false;
	}
	else if(!this->isReverseFlow())
	{
		this->initError();
		return false;
	}
	//TODO:  insert pre-empt code for when wait is called earlier.
	else if(this->getTank()==FULL)
	{
		this->initIdle();
	}

	return true;
}
inline int Controller::loopFill()
{
	if(this->getTank()==TANK_ERROR)
	{
		this->initError();
		return false;
	}
	else if(!timers::delay1)
	{
		return true;
	}
	else if(!this->isReverseFlow())
	{
		this->initError();
		return false;
	}
	else if(this->getTank()==FULL)
	{
		this->initIdle();
	}

	return true;
}

inline int Controller::loopForceFill()
{
	return true;
}

inline int Controller::loopPumping()
{
	if(this->getTank()==TANK_ERROR)
	{
		this->initError();
		return false;
	}
	else if(this->rig.getPumpErrStatus())
	{
		this->initError();
		return false;
	}
	else if(this->getTank()==EMPTY)
	{
		this->initIdle();
	}
	else if(!timers::delay1)
	{
		return true;
	}
	else if(!this->rig.getPumpRunning())
	{
		this->initError();
		return false;
	}
	else if(!this->isPressure() && !this->isFlow())
	{
		this->initError();
		return false;
	}

	return true;
}

inline int Controller::loopPressureTrans()
{
	if(this->getTank()==TANK_ERROR)
	{
		this->initError();
		return false;
	}
	else if(this->rig.getPumpErrStatus())
	{
		this->initError();
		return false;
	}
	else if(this->getTank()==EMPTY)
	{
		this->initIdle();
	}
	else if(!this->rig.getPumpRunning())
	{
		this->initError();
		return false;
	}
	else if(!this->isPressure() && !this->isFlow())
	{
		this->initError();
		return false;
	}

	//TODO:  Closed loop control

	return true;
}

inline int Controller::loopPressureHold()
{
	if(this->getTank()==TANK_ERROR)
	{
		this->initError();
		return false;
	}
	else if(this->rig.getPumpErrStatus())
	{
		this->initError();
		return false;
	}
	else if(this->getTank()==EMPTY)
	{
		this->initIdle();
	}
	else if(!this->rig.getPumpRunning())
	{
		this->initError();
		return false;
	}
	else if(!this->isPressure() && !this->isFlow())
	{
		this->initError();
		return false;
	}
	return true;
}
inline int Controller::loopOverride();
inline int Controller::loopError();

inline int Controller::initIdle()
{
	//Precondition check
	if(!(this->state == IDLE || this->state == PRIME3 || this->state == FILL || this->state == FORCEFILL || this->state == PUMPING || this->state == PRESSURE_HOLD || this->state == OVERRIDE))
	{
		return false;	//Cannot enter
	}

	this->state = IDLE;

	//SET STATE
	bool success = true;
	success &= this->rig.stopPumpOnly();
	success &= this->rig.closeInflowValveOnly();
	success &= this->rig.closeOutflowValveOnly();
	success &= this->rig.closeReleaseValveOnly();

	if(!success)
	{
		this->initError();
		return false;
	}
	return true;
}

inline int Controller::initIdlePres()
{
	//Precondition check
	if(!(this->state == IDLE || this->state == PRIME3 || this->state == FILL || this->state == FORCEFILL || this->state == PUMPING || this->state == PRESSURE_HOLD || this->state == OVERRIDE))
	{
		return false;	//Cannot enter
	}

	this->state = IDLE_PRES;

	//SET STATE
	bool success = true;
	success &= this->rig.stopPumpOnly();
	success &= this->rig.closeInflowValveOnly();
	success &= this->rig.closeOutflowValveOnly();
	success &= this->rig.closeReleaseValveOnly();

	if(!success)
	{
		this->initError();
		return false;
	}
	return true;
}


inline int Controller::initPrime1()
{
	//Precondition check
	if(!this->isPressure() || !(this->state == IDLE || this->state == PRIME1))
	{
		return false;	//Cannot enter
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
		return false;	//Error on enter
	}
	return true;
}

inline int Controller::initPrime2()
{
	//Precondition check
	if(!(this->state == PRIME2 || this->state == PRIME1))
	{
		return false;	//Cannot enter
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
		return false;	//Error on enter
	}
	return true;
}
inline int Controller::initPrime3()
{
	//Precondition check
	if( !(this->state == PRIME2 || this->state == PRIME3))
	{
		return false;	//Cannot enter
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
		return false;	//Error on enter
	}
	return true;
}

inline int Controller::initPrime4()
{
	//Precondition check
	if( !(this->state == PRIME4 || this->state == PRIME3))
	{
		return false;	//Cannot enter
	}

	this->state = PRIME4;
	//SET STATE
	bool success = true;
	success &= this->rig.stopPumpOnly();
	success &= this->rig.openInflowValveOnly();
	success &= this->rig.openOutflowValveOnly();
	success &= this->rig.openReleaseValveOnly();


	if(!success)
	{
		this->initError();
		return false;	//Error on enter
	}
	return true;
}

inline int Controller::initFill()
{
	//Precondition check
	if(!this->isPressure() || !(this->state == FILL || this->state == IDLE))
	{
		return false;	//Cannot enter
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
		return false;	//Error on enter
	}
	return true;
}

inline int Controller::initForceFill()
{
	//Precondition check
	if( !(this->state == FORCEFILL || this->state == IDLE))
	{
		return false;	//Cannot enter
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
		return false;	//Error on enter
	}
	return true;
}

inline int Controller::initPumping()
{
	//Precondition check
	if(this->getTank()==EMPTY || !(this->state == PUMPING || this->state == IDLE || this->state == PRESSURE_HOLD))
	{
		return false;	//Cannot enter
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
		return false;	//Error on enter
	}
	return true;
}

inline int Controller::initPressureTrans()
{
	//Precondition check
	if(!(this->state == PUMPING || this->state == PRESSURE_HOLD || this->state == PRESSURE_TRANS))
	{
		return false;	//Cannot enter
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
		return false;	//Error on enter
	}
	return true;
}

inline int Controller::initPressureHold()
{
	//Precondition check
	if(!(this->state == PRESSURE_HOLD || this->state == PRESSURE_TRANS))
	{
		return false;	//Cannot enter
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
		return false;	//Error on enter
	}
	return true;
}

inline int Controller::initOverride()
{
	//Precondition check
	if(!(this->state == OVERRIDE || this->state == IDLE))
	{
		return false;	//Cannot enter
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
		return false;	//Error on enter
	}
	return true;
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
		return false;	//Error on enter
	}
	return true;
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

