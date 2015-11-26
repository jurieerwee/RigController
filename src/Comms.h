/*
 * Comms.h
 *
 *  Created on: 24 Sep 2015
 *      Author: Jurie
 */

#ifndef COMMS_H_
#define COMMS_H_

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <queue>
#include <string>
#include <boost/log/trivial.hpp>

using namespace std;
namespace src = boost::log::sources;

namespace comms {

extern volatile bool terminate;
extern volatile bool restart;
extern src::severity_logger_mt<>& lg;
//extern queue<string> recvQ;
//extern queue<string> transQ;
//extern pthread_cond_t trans_cond;
//extern pthread_mutex_t trans_mut;

int initComms(int port);
void *loop(void* socketfd_);
bool getError();
bool resetError();
void* recv(void *socketComm_);
void* trans(void *socketComm_);

int pushTransmit(string in);
string popRecv();

void terminateComms();
void restartComms();


} /* namespace comms */

#endif /* COMMS_H_ */
