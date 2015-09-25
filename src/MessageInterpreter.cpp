/*
 * MessageInterpreter.cpp
 *
 *  Created on: 25 Sep 2015
 *      Author: Jurie
 */

#include "MessageInterpreter.h"
#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/exception/all.hpp>

#include "Comms.h"

using namespace std;



MessageInterpreter::MessageInterpreter() {
	//State commands
	instrState.insert(pair<string,stateCmd>("prime",primeCMD));
	instrState.insert(pair<string,stateCmd>("wait",waitCMD));
	instrState.insert(pair<string,stateCmd>("fillTank",fillTankCMD));
	instrState.insert(pair<string,stateCmd>("forceFill",forceFillCMD));
	instrState.insert(pair<string,stateCmd>("startPump",pumpCMD));
	instrState.insert(pair<string,stateCmd>("newPressure",newPressureCMD));
	instrState.insert(pair<string,stateCmd>("releaseHold",releaseHoldCMD));
	instrState.insert(pair<string,stateCmd>("clearErr",clearErrCMD));
	instrState.insert(pair<string,stateCmd>("override",overrideCMD));
	//Munual commands
	instrMan.insert(pair<string,manCmd>("startPump",startPumpCMD));
	instrMan.insert(pair<string,manCmd>("stopPump",stopPumpCMD));
	instrMan.insert(pair<string,manCmd>("setPumpPerc",setPumpPercCMD));
	instrMan.insert(pair<string,manCmd>("openInflowValve",openInflowValveCMD));
	instrMan.insert(pair<string,manCmd>("closeInflowValve",closeInflowValveCMD));
	instrMan.insert(pair<string,manCmd>("openOutflowValve",openOutflowValveCMD));
	instrMan.insert(pair<string,manCmd>("closeOutflowValve",closeOutflowValveCMD));
	instrMan.insert(pair<string,manCmd>("openReleaseValve",openReleaseValveCMD));
	instrMan.insert(pair<string,manCmd>("closeReleaseValve",closeReleaseValveCMD));
	instrMan.insert(pair<string,manCmd>("disableOverride",disableOverrideCMD));
	//Set commands
	instrSet.insert(pair<string,setCmd>("resetCounters",resetCountersCMD));
	instrSet.insert(pair<string,setCmd>("resetWaitState",resetWaitStateCMD));
	instrSet.insert(pair<string,setCmd>("setPressure",setPressureCMD));


}

MessageInterpreter::~MessageInterpreter() {
	// TODO Auto-generated destructor stub
}

int MessageInterpreter::interpret(string in)
{
	using boost::property_tree::ptree;
	ptree 	pt;
	string 	reply;//JSON reply string
	long 	msgNumber = -1;
	string	type;
	string 	instruction;
	istringstream ss(in);
	bool failed = false;

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
	//reply.put("reply.id",msgNumber);
	reply = "{\"reply\":{\"id\":" + to_string(msgNumber) + ",";

	if(failed)
	{
		//DO NOTHING
	}
	else if(type.compare("stateCMD")==0)
	{
		switch(instrState[instruction])
		{
		case primeCMD:
			cout << "Prime pump\n";
			reply += "\"code\":1,";
			break;

		default:
			failed = true;
			break;
		}
	}
	else if(type.compare("manualCMD")==0)
	{
		switch(instrMan[instruction])
		{
		default:
			failed = true;
			break;
		}
	}
	else if(type.compare("setCMD")==0)
	{
		switch(instrSet[instruction])
		{
		default:
			failed = true;
			break;
		}
	}

	reply += "\"success\":" + to_string(!failed) + "}}\n";	//TODO: find a valid way to print TRUE/FALSE instread of 0 or 1

	comms::pushTransmit(reply);


	return 1;
}
