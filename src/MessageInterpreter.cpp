/*
 * MessageInterpreter.cpp
 *
 *  Created on: 25 Sep 2015
 *      Author: Jurie
 */


#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/exception/all.hpp>

#include "MessageInterpreter.h"
#include "Controller.h"
#include "Comms.h"
#include "Timers.h"


using namespace std;



MessageInterpreter::MessageInterpreter() {
	//State commands
	instrState.insert(pair<string,stateCmd>("prime",primeCMD));
	instrState.insert(pair<string,stateCmd>("idle",idleCMD));
	instrState.insert(pair<string,stateCmd>("fillTank",fillTankCMD));
	instrState.insert(pair<string,stateCmd>("forceFill",forceFillCMD));
	instrState.insert(pair<string,stateCmd>("startPump",pumpCMD));
	instrState.insert(pair<string,stateCmd>("newPressure",newPressureCMD));
	instrState.insert(pair<string,stateCmd>("releaseHold",releaseHoldCMD));
	instrState.insert(pair<string,stateCmd>("clearErr",clearErrCMD));
	instrState.insert(pair<string,stateCmd>("override",overrideCMD));
	instrState.insert(pair<string,stateCmd>("disableOverride",disableOverrideCMD));
	instrState.insert(pair<string,stateCmd>("terminate",terminateCMD));
	//Munual commands
	instrMan.insert(pair<string,manCmd>("startPump",startPumpCMD));
	instrMan.insert(pair<string,manCmd>("stopPump",stopPumpCMD));
	instrMan.insert(pair<string,manCmd>("openInflowValve",openInflowValveCMD));
	instrMan.insert(pair<string,manCmd>("closeInflowValve",closeInflowValveCMD));
	instrMan.insert(pair<string,manCmd>("openOutflowValve",openOutflowValveCMD));
	instrMan.insert(pair<string,manCmd>("closeOutflowValve",closeOutflowValveCMD));
	instrMan.insert(pair<string,manCmd>("openReleaseValve",openReleaseValveCMD));
	instrMan.insert(pair<string,manCmd>("closeReleaseValve",closeReleaseValveCMD));
	//Set commands
	instrSet.insert(pair<string,setCmd>("resetCounters",resetCountersCMD));
	instrSet.insert(pair<string,setCmd>("setPressure",setPressureCMD));
	instrSet.insert(pair<string,setCmd>("setPumpPerc",setPumpPercCMD));
	instrSet.insert(pair<string,setCmd>("activateUpdate",activateUpdateCMD));
	//State to string
	stateString.insert(pair<Controller::State,string>(Controller::State::IDLE,"IDLE"));
	stateString.insert(pair<Controller::State,string>(Controller::State::IDLE_PRES,"IDLE_PRES"));
	stateString.insert(pair<Controller::State,string>(Controller::State::PRIME1,"PRIME1"));
	stateString.insert(pair<Controller::State,string>(Controller::State::PRIME2,"PRIME2"));
	stateString.insert(pair<Controller::State,string>(Controller::State::PRIME3,"PRIME3"));
	stateString.insert(pair<Controller::State,string>(Controller::State::PRIME4,"PRIME4"));
	stateString.insert(pair<Controller::State,string>(Controller::State::FILL,"FILL"));
	stateString.insert(pair<Controller::State,string>(Controller::State::FORCEFILL,"FORCEFILL"));
	stateString.insert(pair<Controller::State,string>(Controller::State::PUMPING,"PUMPING"));
	stateString.insert(pair<Controller::State,string>(Controller::State::PRESSURE_TRANS,"PRESSURE_TRANS"));
	stateString.insert(pair<Controller::State,string>(Controller::State::PRESSURE_HOLD,"PRESSURE_HOLD"));
	stateString.insert(pair<Controller::State,string>(Controller::State::OVERRIDE,"OVERRIDE"));
	stateString.insert(pair<Controller::State,string>(Controller::State::ERROR,"ERROR"));
	//Pump state to string
	tankString.insert(pair<Controller::TankState,string>(Controller::TankState::EMPTY,"EMPTY"));
	tankString.insert(pair<Controller::TankState,string>(Controller::TankState::TANK_ERROR,"TANK_ERROR"));
	tankString.insert(pair<Controller::TankState,string>(Controller::TankState::FULL,"FULL"));
	tankString.insert(pair<Controller::TankState,string>(Controller::TankState::TRANSIENT,"TRANSIENT"));
}

MessageInterpreter::~MessageInterpreter() {
	// TODO Auto-generated destructor stub
}

inline string boolToString(bool in)
{
	return in?"true":"false";
}

int MessageInterpreter::sendStatus(Controller *ctrlPtr)
{
	if(!this->sendActive || !timers::sendUpdate)
		return false;

	timers::sendUpdate = false;
	Controller &ctrl = *ctrlPtr;
	string msg;

	msg = "{\"update\":{\"status\":";
	msg += "{\"state\":\"" + stateString[ctrl.state] + "\",\"tank\":\"" + tankString[ctrl.getTank()] + "\",\"inflowValve\":" + boolToString(ctrl.rig.getInflowValve()) \
			+ ",\"outflowValve\":" + boolToString(ctrl.rig.getOutflowValve()) + ",\"releaseValve\":" + boolToString(ctrl.rig.getReleaseValve()) \
			+ ",\"pumpRunning\":" + boolToString(ctrl.rig.getPumpRunning()) + ",\"pumpError\":" + boolToString(ctrl.rig.getPumpErrStatus()) \
			+ ",\"pressurised\":" + boolToString(ctrl.isPressure()) + ",\"forwardFlow\":" + boolToString(ctrl.isFlow())  + ",\"reverseFlow\":" + boolToString(ctrl.isReverseFlow()) + "}";
	msg += ",\"runningData\":{\"flowRate\":"+  to_string(ctrl.rig.getFlowMeasure()) + ",\"flowDir\":" + boolToString(ctrl.rig.getSensor_FlowDirection()) \
			+ ",\"pressure\":" + to_string(ctrl.rig.getSensor_Pressure()) + "}";
	msg += ",\"setData\":{\"flowRate\":" + to_string(ctrl.rig.getFlowCounter()) + ", \"pressure\":" + to_string(ctrl.rig.getSensor_Pressure()) + "}}}\n";

	comms::pushTransmit(msg);

	return true;
}

int MessageInterpreter::interpret(Controller *ctrlPtr)
{
	Controller &ctrl = *ctrlPtr;
	using boost::property_tree::ptree;
	ptree 	pt;
	string 	reply;//JSON reply string
	long 	msgNumber = -1;
	string	type;
	string 	instruction;

	string in = comms::popRecv();


	istringstream ss(in);
	bool failed = false;
	bool terminate = false;

	try{
		read_json(ss,pt);
		msgNumber = pt.get<long>("msg.id");
		type	= pt.get<string>("msg.type");
		instruction = pt.get<string>("msg.instr");

	}catch(boost::exception &e)
	{
		if(msgNumber ==-1)
			return -1;
		else	//Message ID has been successfully been extracted
			failed = true;
	}

	//prepare reply
	reply = "{\"reply\":{\"id\":" + to_string(msgNumber) + ",";

	if(failed)
	{
		//DO NOTHING
	}
	else if(type.compare("stateCMD")==0)
	{

		bool aswr = false;
		switch(instrState[instruction])
		{
		case primeCMD:
			aswr = ctrl.changeState(ctrl.State::PRIME1,true);
			break;
		case idleCMD:
			if(ctrl.state == ctrl.State::ERROR)	//From error state, clearErrorCMD must be called.
				aswr = false;
			else
				aswr = ctrl.changeState(ctrl.State::IDLE,true);
			break;
		case fillTankCMD:
			aswr = ctrl.changeState(ctrl.State::FILL,true);
			break;
		case forceFillCMD:
			aswr = ctrl.changeState(ctrl.State::FORCEFILL,true);
			break;
		case pumpCMD:
			aswr = ctrl.changeState(ctrl.State::PUMPING,true);
			break;
		case newPressureCMD:
			aswr = ctrl.changeState(ctrl.State::PRESSURE_TRANS,true);
			break;
		case releaseHoldCMD:
			aswr = ctrl.changeState(ctrl.State::PUMPING,true);
			break;
		case clearErrCMD:
			aswr = ctrl.changeState(ctrl.State::IDLE,true);
			break;
		case overrideCMD:
			aswr = ctrl.changeState(ctrl.State::OVERRIDE,true);
			break;
		case disableOverrideCMD:
			aswr = ctrl.changeState(ctrl.State::IDLE,true);
			break;
		case terminateCMD:
			if(ctrl.state == ctrl.State::IDLE || ctrl.state == ctrl.State::IDLE_PRES)
			{
				aswr = true;
				terminate = true;
			}
			else
			{
				aswr = false;
			}
			break;
		default:
			failed = true;
			break;
		}
		reply += "\"code\":";
		reply += to_string(aswr);
		reply +=",";
	}
	else if(type.compare("manualCMD")==0)
	{
		bool aswr = false;
		//Check whether in override state.
		if(ctrl.state != ctrl.State::OVERRIDE)
		{
			aswr = false;
		}
		else
		{
			switch(instrMan[instruction])
			{
			case startPumpCMD:
				aswr = ctrl.rig.startPumpOnly();
				break;
			case stopPumpCMD:
				aswr = ctrl.rig.stopPumpOnly();
				break;
			case openInflowValveCMD:
				aswr = ctrl.rig.openInflowValveOnly();
				break;
			case closeInflowValveCMD:
				aswr = ctrl.rig.closeInflowValveOnly();
				break;
			case openOutflowValveCMD:
				aswr = ctrl.rig.openOutflowValveOnly();
				break;
			case closeOutflowValveCMD:
				aswr = ctrl.rig.closeOutflowValveOnly();
				break;
			case openReleaseValveCMD:
				aswr = ctrl.rig.openReleaseValveOnly();
				break;
			case closeReleaseValveCMD:
				aswr = ctrl.rig.closeReleaseValveOnly();
				break;
			default:
				failed = true;
				break;
			}
		}
		reply += "\"code\":";
		reply += to_string(aswr);
		reply +=",";
	}
	else if(type.compare("setCMD")==0)
	{
		bool aswr = false;
		try{
			switch(instrSet[instruction])
			{
			case setPumpPercCMD:
				aswr = ctrl.setDesiredPumpPerc(pt.get<double>("msg.percentage"));
				break;
			case setPressureCMD:
				aswr = true;
				ctrl.setPressure = pt.get<double>("msg.pressure");
				break;
			case resetCountersCMD:
				aswr = ctrl.rig.resetFlowMeasuring();
				break;
			case activateUpdateCMD:
				this->sendActive = true;
				aswr = true;
				break;
			default:
				failed = true;
				break;
			}
		}catch(boost::exception &e)
		{
			failed = true;
		}
		reply += "\"code\":";
		reply += to_string(aswr);
		reply +=",";
	}
	else if(type.compare("tester")==0)
	{
		if(instruction.compare("setPressureCMD"))
		{

		}
	}

	reply += "\"success\":" + to_string(!failed) + "}}\n";	//TODO: find a valid way to print TRUE/FALSE instread of 0 or 1

	comms::pushTransmit(reply);


	return terminate?2:1;
}
