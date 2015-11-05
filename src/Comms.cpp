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
#include <boost/log/trivial.hpp>
#include "Logging.h"

namespace logging = boost::log;
namespace src = boost::log::sources;

using namespace std;

namespace comms {

	//src::severity_logger_mt<>& lg;
	volatile bool terminate;
	volatile bool restart;
	volatile int socketfd;
	volatile bool live =false;
	queue<string> recvQ;
	pthread_mutex_t recv_mut;
	queue<string> transQ;
	pthread_cond_t	trans_cond;
	pthread_mutex_t trans_mut;

	//Example from http://www.tutorialspoint.com/unix_sockets/socket_server_example.htm (23/09/2015)
	int initComms(int port)
	{
		//lg = my_logger::get();

		int portno;
		struct sockaddr_in serv_addr;
		socketfd = socket(AF_INET,SOCK_STREAM,0);
		live = false;
		portno = port;

		BOOST_LOG_SEV(my_logger::get(),logging::trivial::info)<<"Connecting on port " << portno;
		if(socketfd<0)
		{
			BOOST_LOG_SEV(my_logger::get(),logging::trivial::error)<< "Error opening socket";
			return -1;
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
			BOOST_LOG_SEV(my_logger::get(),logging::trivial::error) <<("ERROR on binding");
			return -1;
		}

		return socketfd;
	}

	void *loop(void* socketfd_)
	{
		//int socketfd = *(int*)socketfd_;
		int socketComm;
		socklen_t client_len;
		struct sockaddr_in client_addr;
		pthread_t transmitter, receiver;
		int reuse = 1;

		while(!terminate)
		{
			restart = false;
			listen(socketfd,1);
			client_len = sizeof(client_addr);

			socketComm = accept(socketfd, (struct sockaddr *)&client_addr, &client_len);
			setsockopt(socketComm,SOL_SOCKET,SO_REUSEADDR,(char *) &reuse, sizeof(reuse));
			if(socketComm<0)
			{
				//cout << "Error on socket accept\n";
				continue;
			}
			BOOST_LOG_SEV(my_logger::get(),logging::trivial::info) << "Conneted";
			//Create threads
			pthread_create(&transmitter,NULL, &trans, (void*)(&socketComm));
			pthread_create(&receiver,NULL, &recv, (void*)(&socketComm));
			live = true;
			//Join threads
			pthread_join(transmitter,NULL);
			live = false;
			shutdown(socketComm,SHUT_RDWR);	//Closing socket here ensures that thread exits read
			close(socketComm);
			pthread_join(receiver,NULL);

			BOOST_LOG_SEV(my_logger::get(),logging::trivial::info) << "Comm threads joined";

		}
		close(socketfd);
		return NULL;
	}

	void* recv(void* socketComm_)
	{
		int socketComm = *(int*)(socketComm_);
		int reply = 0;
		char buff[255];
#ifdef DEBUG
		BOOST_LOG_SEV(my_logger::get(),logging::trivial::debug) << "Receive thread started";
#endif
		while(!terminate && !restart)
		{
			memset(buff, 0, sizeof(buff));
			reply = read(socketComm,(void*)buff,254);
			if(reply>0)
			{
				string temp(buff);
				pthread_mutex_lock(&recv_mut);
				recvQ.push(temp);
				pthread_mutex_unlock(&recv_mut);
#ifdef DEBUG
				BOOST_LOG_SEV(my_logger::get(),logging::trivial::debug) << "Msg added to recvQ";
#endif
			}
			else if(reply==0 || reply==-1)
			{
				//if(errno != EAGAIN && errno != EWOULDBLOCK && errno != EINPROGRESS)
				//{
					restartComms();
					return NULL;
				//}
			}
			if( recvQ.size()>10)
			{
				terminateComms();
				return NULL;
			}
		}
#ifdef DEBUG
		BOOST_LOG_SEV(my_logger::get(),logging::trivial::debug) << "Receive thread ending";
#endif

		return NULL;
	}

	void* trans(void* socketComm_)
	{
		int socketComm = *(int*)(socketComm_);
		char buff[1024];
		int reply = 0;
		string temp;
		while(!terminate && !restart)
		{

			while(transQ.size()>0)
			{

				pthread_mutex_lock(&trans_mut);
				temp = transQ.front();
				transQ.pop();
				pthread_mutex_unlock(&trans_mut);

				if (temp.length()>1023)
				{
					BOOST_LOG_SEV(my_logger::get(),logging::trivial::error) << "Transmit message too large";
					terminateComms();
					return NULL;
				}
				strncpy(buff,temp.c_str(),1024);
				buff[temp.length()] = '\0';
				//cout << "Message to be sent: " << buff << "\n";
				reply = write(socketComm,(void*)buff,1024);
				if (reply < 0)
				{
					BOOST_LOG_SEV(my_logger::get(),logging::trivial::error) << "Transmit error";
					restartComms();
					return NULL;
				}

			}
			//cout << "Transmit thread sleep\n";
			pthread_mutex_lock(&trans_mut);
			while(0!=pthread_cond_wait(&trans_cond,&trans_mut));
			pthread_mutex_unlock(&trans_mut);
			//cout << "Transmit thread awakes\n";
		}

		return NULL;
	}

	int pushTransmit(string in)
	{
		if(!live)
			return 0;

		pthread_mutex_lock(&trans_mut);
		transQ.push(in);
		pthread_mutex_unlock(&trans_mut);
		pthread_cond_signal(&comms::trans_cond);
		//cout << "Transmit msg pushed\n";
		return 1;
	}

	string popRecv()
	{
		if(recvQ.empty())
			throw 0;

		string temp;
		pthread_mutex_lock(&recv_mut);
		temp = recvQ.front();
		recvQ.pop();
		pthread_mutex_unlock(&recv_mut);

		return temp;
	}

	void terminateComms()
	{
		terminate = true;
		shutdown(socketfd,SHUT_RDWR);
		close(socketfd);
		pthread_cond_signal(&comms::trans_cond);
	}

	void restartComms()
	{
		//Empty queue
		pthread_mutex_lock(&trans_mut);
		while(!transQ.empty())
		{
			transQ.pop();
		}
		pthread_mutex_unlock(&trans_mut);
		pthread_mutex_lock(&recv_mut);
		while(!transQ.empty())
		{
			transQ.pop();
		}
		pthread_mutex_unlock(&recv_mut);
		restart = true;
		pthread_cond_signal(&comms::trans_cond);
	}

} /* namespace comms */
