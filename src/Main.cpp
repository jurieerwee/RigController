/*
 * Main.cpp
 *
 *  Created on: 23 Sep 2015
 *      Author: Jurie
 */

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <queue>

using namespace std;

volatile bool terminate;
volatile queue<string> recvQ;
volatile queue<string> sendQ;

 
int main(int argc, const char* argv[])
{


	//Example from http://www.tutorialspoint.com/unix_sockets/socket_server_example.htm (23/09/2015)
	int socketfd, portno, socketComm, client_len;
	struct sockaddr_in server_add, client_add;
	socketfd = socket(AF_INET,SOCK_STEAM,0);

	if(socketfd<0)
	{
		cout<< "Error opening socket\n";
		return 1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	if (bind(socketfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		cout <<("ERROR on binding\n");
		return 1;
	}
	//TODO: spawn control thread
	do
	{
		listen(socketfd,1);
		client_len = sizeof(client_add);
		socketComm = accept(socketfd, (struct sockeaddr *)&client_add, &client_len);
		if(socketComm<0)
		{
			cout << "Error on socket accept\n";
			continue;
		}

		//TODO: Spawn threads


		//TODO: bind threads


	}while(!terminate);

}
