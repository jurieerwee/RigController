/*
 * Controller.cpp
 *
 *  Created on: 28 Sep 2015
 *      Author: Jurie
 */

#include "Controller.h"
#include "Rig.h"
#include "Timers.h"
#include "Comms.h"
//#include "MessageInterpreter.h"

#include <string>

#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/log/trivial.hpp>

namespace po = boost::program_options;
namespace src = boost::log::sources;

using namespace std;

Controller::Controller(po::variables_map& vm_) : lg(my_logger::get()), rig((vm_)) , presThresh(vm_["pressureThreshold"].as<double>()), pressSettledTolerance(vm_["pressSettledTolerance"].as<double>()),\
		pressSettledCount(vm_["pressSettledCount"].as<int>()), kp(vm_["kp"].as<double>()),ki(vm_["ki"].as<double>())
{
	// TODO Auto-generated constructor stub
	BOOST_LOG_SEV(this->lg,logging::trivial::info) << "Pressure threshold set to: " << this->presThresh ;

	this->stateString.insert(pair<State,string>(State::IDLE,"IDLE"));
	this->stateString.insert(pair<State,string>(State::IDLE_PRES,"IDLE_PRES"));
	this->stateString.insert(pair<State,string>(State::PRIME1,"PRIME1"));
	this->stateString.insert(pair<State,string>(State::PRIME2,"PRIME2"));
	this->stateString.insert(pair<State,string>(State::PRIME3,"PRIME3"));
	this->stateString.insert(pair<State,string>(State::PRIME4,"PRIME4"));
	this->stateString.insert(pair<State,string>(State::FILL,"FILL"));
	this->stateString.insert(pair<State,string>(State::FORCEFILL,"FORCEFILL"));
	this->stateString.insert(pair<State,string>(State::PUMPING,"PUMPING"));
	this->stateString.insert(pair<State,string>(State::PRESSURE_TRANS,"PRESSURE_TRANS"));
	this->stateString.insert(pair<State,string>(State::PRESSURE_HOLD,"PRESSURE_HOLD"));
	this->stateString.insert(pair<State,string>(State::OVERRIDE,"OVERRIDE"));
	this->stateString.insert(pair<State,string>(State::ERROR,"ERROR"));

	this->initIdle();
}

Controller::~Controller() {
	// TODO Auto-generated destructor stub
	//State to string

}




int Controller::loop()
{
	//Note: Interpret called in ctrlTread loop.  This avoids circular dependence.

	if(this->rig.getEmerBtn())
	{
		changeState(ERROR,false);
	}

	if(timers::controllerPulse)
	{
		timers::controllerPulse = false;
		this->rig.forceSensorUpdate();
		if(this->dump)
		{
			this->dataDump();
		}


		if(this->state == PRESSURE_TRANS)
		{
			this->piControl();
		}

	}

	if(timers::flowTimeout)
	{
		this->rig.flowZeroPulse();
		timers::reset_flowTimeout();
	}

	if(this->rig.getPumpPerc()!=this->setPercentage)	//If desired set percentage changed, change pump speed
		this->rig.setPumpSpeed(this->setPercentage);

	switch(this->state)
	{
	case IDLE:
		this->loopIdle();
		break;
	case IDLE_PRES:
		this->loopIdlePres();
		break;
	case PRIME1:
		this->loopPrime1();
		break;
	case PRIME2:
		this->loopPrime2();
		break;
	case PRIME3:
		this->loopPrime3();
		break;
	case PRIME4:
		this->loopPrime4();
		break;
	case FILL:
		this->loopFill();
		break;
	case FORCEFILL:
		this->loopForceFill();
		break;
	case PUMPING:
		this->loopPumping();
		break;
	case PRESSURE_TRANS:
		this->loopPressureTrans();
		break;
	case PRESSURE_HOLD:
		this->loopPressureHold();
		break;
	case OVERRIDE:
		this->loopOverride();
		break;
	case ERROR:
		this->loopError();
		break;
	default:
		this->changeState(ERROR,false);
		break;
	}

	return true;
}

int Controller::changeState(State newState, bool cmd)
{
	BOOST_LOG_SEV(this->lg,logging::trivial::info) << "ChangeState request: " << this->stateString[newState] << ", cmd=" << cmd;

	switch(newState)
	{
	case ERROR:
		return initError();
	case IDLE:
		if ((this->state == IDLE_PRES || this->state == FILL || this->state == PRIME4 || this->state == PUMPING ||\
				this->state == PRESSURE_TRANS || this->state == PRESSURE_HOLD ||this->state == OVERRIDE ||this->state == FORCEFILL) || \
				(this->state == PRIME3 && !cmd) || (this->state==ERROR && !this->rig.getEmerBtn() && cmd))
		{
			return this->initIdle();
		}
		else
			return false;
		break;

	case IDLE_PRES:
		if (this->state == IDLE && !cmd)
			return this->initIdlePres();
		else
			return false;
		break;
	case PRIME1:
		if (this->state == IDLE_PRES && cmd)
			return this->initPrime1();
		else
			return false;
		break;
	case PRIME2:
		if (this->state == PRIME1 && !cmd)
			return this->initPrime2();
		else
			return false;
		break;
	case PRIME3:
		if (this->state == PRIME2 && !cmd)
			return this->initPrime3();
		else
			return false;
		break;
	case PRIME4:
		if (this->state == PRIME3 && !cmd)
			return this->initPrime4();
		else
			return false;
		break;
	case FILL:
		if (this->state == IDLE_PRES && cmd)
			return this->initFill();
		else
			return false;
		break;
	case FORCEFILL:
		if ((this->state == IDLE_PRES || this->state ==IDLE) && cmd)
			return this->initForceFill();
		else
			return false;
		break;
	case PUMPING:
		if ((this->state == IDLE_PRES || this->state == this->IDLE || this->state == this->PRESSURE_HOLD) && cmd)
			return this->initPumping();
		else
			return false;
		break;
	case PRESSURE_TRANS:
		if ((this->state == PUMPING || this->state == this->PRESSURE_HOLD) && cmd)
			return this->initPressureTrans();
		else
			return false;
		break;
	case PRESSURE_HOLD:
		if ((this->PRESSURE_TRANS) &&!cmd)
			return this->initPressureHold();
		else
			return false;
		break;
	case OVERRIDE:
		if ((this->state == IDLE_PRES ||this->state ==  this->IDLE) && cmd)
			return this->initOverride();
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
	if(this->isPressure())
	{
		this->changeState(IDLE_PRES,false);
	}
	else if(!timers::pumpStop)
	{
		return true;
	}
	else if(this->rig.getPumpRunning())
	{
		this->changeState(ERROR,false);
		return false;
	}

	return true;
}

inline int Controller::loopIdlePres()
{
	if(!this->isPressure())
	{
		this->changeState(IDLE,false);
	}
	else if(!timers::pumpStop)
	{
		return true;
	}
	else if(this->rig.getPumpRunning())
	{
		this->changeState(ERROR,false);
		return false;
	}

	return true;
}

inline int Controller::loopPrime1()
{
	if(this->getTank()==TANK_ERROR)
	{
		this->changeState(ERROR,false);
		return false;
	}
	else if(!timers::delay1)
	{
		return true;
	}
	else if(!this->isReverseFlow())
	{
		this->changeState(ERROR,false);
		return false;
	}
	else
	{
		this->changeState(PRIME2,false);
	}

	return true;

}

inline int Controller::loopPrime2()
{
	if(this->getTank()==TANK_ERROR)
	{
		this->changeState(ERROR,false);
		return false;
	}
	else if(!this->isReverseFlow())
	{
		this->changeState(ERROR,false);
		return false;
	}
	else if(!timers::delay1)
	{
		return true;
	}

	else
	{
		this->changeState(PRIME3,false);;
	}
	return true;
}

inline int Controller::loopPrime3()
{
	if(this->getTank()==TANK_ERROR)
	{
		this->changeState(ERROR,false);
		return false;
	}
	else if(!this->isReverseFlow())
	{
		this->changeState(ERROR,false);
		return false;
	}
	else if(this->getTank()==FULL)
	{
		this->changeState(IDLE,false);
	}
	else if(this->getTank()==EMPTY)
	{
		return true;
	}
	else if(timers::delay30)
	{
		this->changeState(PRIME4,false);
	}

	return true;
}

inline int Controller::loopPrime4()
{
	if(this->getTank()==TANK_ERROR)
	{
		this->changeState(ERROR,false);
		return false;
	}
	else if(!this->isReverseFlow())
	{
		this->changeState(ERROR,false);
		return false;
	}
	else if(this->getTank()==FULL)
	{
		this->changeState(IDLE,false);
	}

	return true;
}
inline int Controller::loopFill()
{
	if(this->getTank()==TANK_ERROR)
	{
		this->changeState(ERROR,false);
		return false;
	}
	else if(!timers::delay1)
	{
		return true;
	}
	else if(!this->isReverseFlow())
	{
		this->changeState(ERROR,false);
		return false;
	}
	else if(this->getTank()==FULL)
	{
		this->changeState(IDLE,false);
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
		this->changeState(ERROR,false);
		return false;
	}
	else if(this->rig.getPumpErrStatus())
	{
		this->changeState(ERROR,false);
		return false;
	}
	else if(this->getTank()==EMPTY)
	{
		this->changeState(IDLE,false);
	}
	else if(!timers::delay1)
	{
		return true;
	}
	else if(!this->rig.getPumpRunning())
	{
		this->changeState(ERROR,false);
		return false;
	}
	else if(!this->isPressure() && !this->isForwardFlow())
	{
		this->changeState(ERROR,false);
		return false;
	}

	return true;
}

inline int Controller::loopPressureTrans()
{
	if(this->getTank()==TANK_ERROR)
	{
		this->changeState(ERROR,false);
		return false;
	}
	else if(this->rig.getPumpErrStatus())
	{
		this->changeState(ERROR,false);
		return false;
	}
	else if(this->getTank()==EMPTY)
	{
		this->changeState(IDLE,false);
	}
	else if(!this->rig.getPumpRunning())
	{
		this->changeState(ERROR,false);
		return false;
	}
	else if(!this->isPressure() && !this->isForwardFlow())
	{
		this->changeState(ERROR,false);
		return false;
	}

	//TODO: PressureReached??
	//TODO:  Closed loop control



	return true;
}

inline int Controller::loopPressureHold()
{
	if(this->getTank()==TANK_ERROR)
	{
		this->changeState(ERROR,false);
		return false;
	}
	else if(this->rig.getPumpErrStatus())
	{
		this->changeState(ERROR,false);
		return false;
	}
	else if(this->getTank()==EMPTY)
	{
		this->changeState(IDLE,false);
	}
	else if(!this->rig.getPumpRunning())
	{
		this->changeState(ERROR,false);
		return false;
	}
	else if(!this->isPressure() && !this->isForwardFlow())
	{
		this->changeState(ERROR,false);
		return false;
	}
	return true;
}

inline int Controller::loopOverride()
{
	return true;
}

inline int Controller::loopError()
{
	return true;
}

inline int Controller::initIdle()
{
	this->state = IDLE;

	//SET STATE
	bool success = true;
	success &= this->rig.stopPumpOnly();
	success &= this->rig.closeInflowValveOnly();
	success &= this->rig.closeOutflowValveOnly();
	success &= this->rig.closeReleaseValveOnly();

	timers::reset_pumpStop();	//Start 1 second delay

	if(!success)
	{
		this->changeState(ERROR,false);
		return false;
	}
	return true;
}

inline int Controller::initIdlePres()
{
	this->state = IDLE_PRES;

	//SET STATE
	bool success = true;
	success &= this->rig.stopPumpOnly();
	success &= this->rig.closeInflowValveOnly();
	success &= this->rig.closeOutflowValveOnly();
	success &= this->rig.closeReleaseValveOnly();

	//timers::reset_delay1();	//Start 1 second delay

	if(!success)
	{
		this->changeState(ERROR,false);
		return false;
	}
	return true;
}


inline int Controller::initPrime1()
{
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
		this->changeState(ERROR,false);
		return false;	//Error on enter
	}
	return true;
}

inline int Controller::initPrime2()
{
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
		this->changeState(ERROR,false);
		return false;	//Error on enter
	}
	return true;
}
inline int Controller::initPrime3()
{
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
		this->changeState(ERROR,false);
		return false;	//Error on enter
	}
	return true;
}

inline int Controller::initPrime4()
{
	this->state = PRIME4;
	//SET STATE
	bool success = true;
	success &= this->rig.stopPumpOnly();
	success &= this->rig.openInflowValveOnly();
	success &= this->rig.openOutflowValveOnly();
	success &= this->rig.openReleaseValveOnly();


	if(!success)
	{
		this->changeState(ERROR,false);
		return false;	//Error on enter
	}
	return true;
}

inline int Controller::initFill()
{
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
		this->changeState(ERROR,false);
		return false;	//Error on enter
	}
	return true;
}

inline int Controller::initForceFill()
{
	this->state = FORCEFILL;
		//SET STATE
	bool success = true;
	success &= this->rig.stopPumpOnly();
	success &= this->rig.openInflowValveOnly();
	success &= this->rig.closeOutflowValveOnly();
	success &= this->rig.closeReleaseValveOnly();

	if(!success)
	{
		this->changeState(ERROR,false);
		return false;	//Error on enter
	}
	return true;
}

inline int Controller::initPumping()
{
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
		this->changeState(ERROR,false);
		return false;	//Error on enter
	}
	return true;
}

inline int Controller::initPressureTrans()
{
	this->state = PRESSURE_TRANS;

	//SET STATE
	bool success = true;
	success &= this->rig.startPumpOnly();
	success &= this->rig.closeInflowValveOnly();
	success &= this->rig.openOutflowValveOnly();
	success &= this->rig.closeReleaseValveOnly();

	this->pressSettledCounter = 0; //Reset

	this->ui = this->setPercentage;

	if(!success)
	{
		this->changeState(ERROR,false);
		return false;	//Error on enter
	}
	return true;
}

inline int Controller::initPressureHold()
{
	this->state = PRESSURE_HOLD;

	//SET STATE
	bool success = true;
	success &= this->rig.startPumpOnly();
	success &= this->rig.closeInflowValveOnly();
	success &= this->rig.openOutflowValveOnly();
	success &= this->rig.closeReleaseValveOnly();

	if(!success)
	{
		this->changeState(ERROR,false);
		return false;	//Error on enter
	}
	return true;
}

inline int Controller::initOverride()
{
	this->state = OVERRIDE;

	//SET STATE
	bool success = true;
	success &= this->rig.stopPumpOnly();
	success &= this->rig.closeInflowValveOnly();
	success &= this->rig.closeOutflowValveOnly();
	success &= this->rig.closeReleaseValveOnly();

	if(!success)
	{
		this->changeState(ERROR,false);
		return false;	//Error on enter
	}
	return true;
}

inline int Controller::initError()
{
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

inline bool Controller::piControl()
{
	double err = this->setPressure - this->rig.getSensor_Pressure();

	this->ui += err * this->ki;

	if(this->ui >1)
		this->ui = 1;
	else if(this->ui <0)
			this->ui = 0;

	double u = this->ui + this->kp * err;

	if(u >1)
		u = 1;
	else if(u <0)
		u = 0;

	this->setPercentage = u;

	if(err <-0.1)
	{
		this->rig.openReleaseValveOnly();
	}

	if(this->rig.getReleaseValve() && err>0)
		this->rig.closeReleaseValveOnly();


	return true;
}

inline bool Controller::isPressure()	//Check whether pressure is high enough
{

	return (this->rig.getSensor_Pressure() > this->presThresh);
}

inline Controller::TankState Controller::getTank()	//Translate two tank sensors to a state
{
	//cout << "Empty: " << this->rig.getSensor_EmptyTank() << ", Full:" << (this->rig.getSensor_FullTank()) << "\n";

	//return (Controller::TankState)( (unsigned int)this->rig.getSensor_EmptyTank() + (unsigned int)(this->rig.getSensor_FullTank())<<1);	//00= transient, 01 = EMPTY, 10= FULL, 11 = ERROR*/

	if(!this->rig.getSensor_EmptyTank() && !this->rig.getSensor_FullTank())
	{
		return TankState::TRANSIENT;
	}
	else if(!this->rig.getSensor_EmptyTank() && this->rig.getSensor_FullTank())
	{
		return TankState::FULL;
	}
	else if(this->rig.getSensor_EmptyTank() && !this->rig.getSensor_FullTank())
	{
		return TankState::EMPTY;
	}
	else
	{
		return TankState::TANK_ERROR;
	}


}

inline bool Controller::isReverseFlow()
{
	return (this->rig.getFlowMeasure()>0 && !this->rig.getSensor_FlowDirection());
}

inline bool Controller::isForwardFlow()
{
	return (this->rig.getFlowMeasure() >0 && this->rig.getSensor_FlowDirection());
}

int Controller::setDesiredPumpPerc(double in)
{
	if(this->state != PRESSURE_TRANS && this->state != PRESSURE_HOLD && in<=1. && in>=0)
	{
		this->setPercentage = in;
		return true;
	}
	else
	{
		return false;
	}
}

bool Controller::initDataDump()
{
	this->dataDumpFile.open("dataDump.csv");
	this->dump = true;
	this->dataDumpFile << "Pump Perc; Pressure; Flow rate; Release valve\n";

	BOOST_LOG_SEV(this->lg,logging::trivial::info) << "Data dump initiated";

	return true;
}
inline bool Controller::dataDump()
{
	this->dataDumpFile << to_string(this->rig.getPumpPerc()) << ";" << to_string(this->rig.getSensor_Pressure()) << ";" << to_string(this->rig.getFlowMeasure()) << ";"  << to_string(this->rig.getReleaseValve()) << "\n";

	return true;
}
bool Controller::stopDataDump()
{
	this->dump = false;
	this->dataDumpFile.close();

	return true;
}
