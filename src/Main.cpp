/*
 * Main.cpp
 *
 *  Created on: 23 Sep 2015
 *      Author: Jurie
 */

#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <queue>
#include <pthread.h>
#include <string>
#include <sstream>

#include <boost/log/trivial.hpp>
#include <boost/log/sources/severity_feature.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/manipulators/to_log.hpp>

#include "Comms.h"
#include "Controller.h"
#include "MessageInterpreter.h"
#include "Logging.h"
#include "Timers.h"
#include "Options.h"



//namespace po = boost::program_options;
using namespace std;



void * test(void* args);
void *ctrlThread(void* args);


 
int main(int argc, const char* argv[])
{

	char* endprt;
	int portno = (int)strtol(argv[1],&endprt,10);

	Options opt;
	opt.initOptions();

	initLogger();
	timers::init();
	comms::initComms(portno);

	//TODO: spawn control thread
	pthread_t ctrl;
	pthread_create(&ctrl,NULL,&ctrlThread,(void *)(&opt.getVM()));

	comms::loop(&portno);	//spawns and joins threads also.

	//Join control tread
	pthread_join(ctrl,NULL);

	exit(0);
}


/*void * test(void* args)
{
	char cmd;
	string msg;

	MessageInterpreter mi;

	while(!comms::terminate)
	{
		cout << "\nr: readQ \nw: writeQ \nt: terminate\n";
		cin >> cmd;

		switch(cmd)
		{
		case 'r':
			try
			{
				cout << comms::popRecv().c_str() << "\n";
			}
			catch(int e)
			{
				cout << "Queue is empty\n";
			}
			break;
		case 'w':
			cin >> msg;
			cout << "Added to Q: "<<comms::pushTransmit(msg) << "\n";
			break;
		case 't':
			comms::terminateComms();
			break;
		case 'i':
			try
			{
				string temp = comms::popRecv();
				//istringstream ss(temp);
				mi.interpret(temp);
			}
			catch(int e)
			{
				cout << "Queue is empty\n";
			}
			break;
		default:
			cout << "Invalid input\n";

		}
	}
	return NULL;

}*/

void *ctrlThread(void* vm_)
{
	po::variables_map& vm = *(po::variables_map*)(vm_);
	MessageInterpreter mi;
	Controller ctrler(vm);
	bool terminate = false;

	while(!terminate)
	{
		ctrler.loop();
		try
		{
			terminate = (mi.interpret(&ctrler)==2);
		}
		catch(int e)
		{
			if(0!=e)
				throw e;
		}
	}
	comms::terminateComms();
	return NULL;
}


