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

using namespace std;
class MessageInterpreter {
public:
	MessageInterpreter();
	virtual ~MessageInterpreter();

	int interpret(string in);

private:

	map<string,int>	instrMap;
};

#endif /* SRC_MESSAGEINTERPRETER_H_ */
