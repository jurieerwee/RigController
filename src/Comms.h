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

using namespace std;

namespace comms {

extern volatile bool terminate;
extern volatile bool restart;
extern queue<string> recvQ;
extern queue<string> transQ;
extern pthread_cond_t trans_cond;
extern pthread_mutex_t trans_mut;

void* recv(void *socketComm_);
void* trans(void *socketComm_);

void terminateComms();


} /* namespace comms */

#endif /* COMMS_H_ */
