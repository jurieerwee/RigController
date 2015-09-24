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


	//Example from http://www.tutorialspoint.com/unix_sockets/socket_server_example.htm (23/09/2015)
	int socketfd, portno, socketComm;
	socklen_t client_len;
	struct sockaddr_in serv_addr, client_addr;
	socketfd = socket(AF_INET,SOCK_STREAM,0);

	char* endprt;
	portno = (int)strtol(argv[1],&endprt,10);

	cout<<"Connecting on port " << portno << "\n";

	if(socketfd<0)
	{
		cout<< "Error opening socket\n";
		return 1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	int reuse = 1;
	struct linger lin = {1,2};
	setsockopt(socketfd,SOL_SOCKET,SO_REUSEADDR,(char *) &reuse, sizeof(reuse));
	setsockopt(socketfd,SOL_SOCKET,SO_LINGER,(char *) &lin, sizeof(lin));
	if (bind(socketfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		cout <<("ERROR on binding\n");
		return 1;
	}
	//TODO: spawn control thread
	pthread_t ctrl;
	pthread_create(&ctrl,NULL,&test,NULL);

	pthread_t transmitter, receiver;
	do
	{
		comms::restart = false;
		listen(socketfd,1);
		client_len = sizeof(client_addr);
		struct timeval tv = {1,0};

		socketComm = accept(socketfd, (struct sockaddr *)&client_addr, &client_len);
		setsockopt(socketComm,SOL_SOCKET,SO_REUSEADDR,(char *) &reuse, sizeof(reuse));
		if(socketComm<0)
		{
			cout << "Error on socket accept\n";
			continue;
		}

		cout << "Conneted\n";
		//Create threads
		pthread_create(&transmitter,NULL, &comms::trans, (void*)(&socketComm));
		pthread_create(&receiver,NULL, &comms::recv, (void*)(&socketComm));
		//Join threads
		pthread_join(transmitter,NULL);
		shutdown(socketComm,SHUT_RDWR);	//Closing socket here ensures that thread exits read
		close(socketComm);
		close(socketfd);

		pthread_join(receiver,NULL);


	}while(!comms::terminate);

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
			if (comms::recvQ.size()>0)
			{
				cout << comms::recvQ.front().c_str() << "\n";
				comms::recvQ.pop();
			}
			break;
		case 'w':
			cin >> msg;
			pthread_mutex_lock(&comms::trans_mut);
			comms::transQ.push(msg);
			pthread_mutex_unlock(&comms::trans_mut);
			pthread_cond_signal(&comms::trans_cond);
			break;
		case 't':
			comms::terminate = true;
			pthread_cond_signal(&comms::trans_cond);
			break;
		default:
			cout << "Invalid input\n";

		}
	}
	return NULL;

}
