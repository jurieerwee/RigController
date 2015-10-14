/*
 * MessageInterpreter.h
 *
 *  Created on: 25 Sep 2015
 *      Author: Jurie
 */

#ifndef SRC_MESSAGEINTERPRETER_H_
#define SRC_MESSAGEINTERPRETER_H_
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "Controller.h"
using namespace std;
class MessageInterpreter {
public:
	MessageInterpreter();
	virtual ~MessageInterpreter();

	int interpret(Controller *ctrlPtr);

private:
	enum stateCmd {primeCMD,idleCMD,fillTankCMD,forceFillCMD,pumpCMD,\
		newPressureCMD,releaseHoldCMD,clearErrCMD,overrideCMD,disableOverrideCMD};
	map<string,stateCmd>	instrState;
	enum manCmd {startPumpCMD,stopPumpCMD,openInflowValveCMD,\
		closeInflowValveCMD,openOutflowValveCMD,closeOutflowValveCMD,openReleaseValveCMD,closeReleaseValveCMD};
	map<string,manCmd>		instrMan;
	enum setCmd {resetCountersCMD,setPumpPercCMD,setPressureCMD};
	map<string,setCmd>		instrSet;

};

#endif /* SRC_MESSAGEINTERPRETER_H_ */
