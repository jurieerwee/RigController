/*
 * Comms.cpp
 *
 *  Created on: 24 Sep 2015
 *      Author: Jurie
 */

#include "Comms.h"

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <queue>
#include <pthread.h>
#include <string>
#include <iostream>

using namespace std;



namespace comms {

	volatile bool terminate;
	volatile bool restart;
	queue<string> recvQ;
	queue<string> transQ;
	pthread_cond_t	trans_cond;
	pthread_mutex_t trans_mut;

	void* recv(void* socketComm_)
	{
		int socketComm = *(int*)(socketComm_);
		int reply = 0;
		char buff[255];

		while(!terminate && !restart)
		{
			reply = read(socketComm,(void*)buff,254);
			if(reply>0)
			{
				string temp(buff);
				recvQ.push(temp);
			}
			if(reply==0 || recvQ.size()>10)
			{
				terminateComms();
				return NULL;
			}
			else if(reply==-1)
			{
				restart = true;
				return NULL;
			}
		}

		return NULL;
	}

	void* trans(void* socketComm_)
	{
		int socketComm = *(int*)(socketComm_);
		char buff[255];
		int reply = 0;

		while(!terminate && !restart)
		{
			cout << "Transmit thread sleep\n";
			pthread_mutex_lock(&trans_mut);
			while(0!=pthread_cond_wait(&trans_cond,&trans_mut));
			pthread_mutex_unlock(&trans_mut);
			cout << "Transmit thread awakes\n";
			while(transQ.size()>0)
			{
				string temp;
				pthread_mutex_lock(&trans_mut);
				temp = transQ.front();
				transQ.pop();
				pthread_mutex_unlock(&trans_mut);

				if (temp.length()>254)
				{
					cout << "Transmit message too large\n";
					terminateComms();
					return NULL;
				}
				strcpy(buff,temp.c_str());
				reply = write(socketComm,(void*)buff,255);
				if (reply < 0)
				{
					cout << "Transmit error\n";
					restart = true;
					return NULL;
				}

			}
		}

		return NULL;
	}

	void terminateComms()
	{
		terminate = true;
		pthread_cond_signal(&comms::trans_cond);
	}

} /* namespace comms */
