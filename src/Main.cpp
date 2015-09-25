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

#include "Comms.h"

using namespace std;

void * test(void* args);

 
int main(int argc, const char* argv[])
{

	char* endprt;
	int portno = (int)strtol(argv[1],&endprt,10);

	comms::initComms(portno);

	//TODO: spawn control thread
	pthread_t ctrl;
	pthread_create(&ctrl,NULL,&test,NULL);

	comms::loop(&portno);

	//Join control tread

	exit(0);
}


void * test(void* args)
{
	char cmd;
	string msg;

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
		default:
			cout << "Invalid input\n";

		}
	}
	return NULL;

}
