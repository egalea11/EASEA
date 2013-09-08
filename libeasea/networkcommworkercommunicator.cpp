/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#include "include/networkcommworkercommunicator.h"
#include <stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>


#define MAXINDSIZE 50000

extern pthread_mutex_t server_mutex;

int NetworkCommWorkerCommunicator::init()
{
    struct sockaddr_in ServAddr; /* Local address */

  
        /* Create socket for incoming connections */
    if ((ServerSocket =  socket(AF_INET,SOCK_DGRAM,0)) < 0) {
		printf("%d\n",socket(AF_INET,SOCK_DGRAM,0));
        printf("Socket create problem.\n"); exit(1);
    }

        /* Construct local address structure */
    
    int tries = 0;
    int port = myself.get_port();
    
    while(tries<5)
    {  
	memset(&ServAddr, 0, sizeof(ServAddr));   /* Zero out structure */
	ServAddr.sin_family = AF_INET;                /* Internet address family */
	ServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
	ServAddr.sin_port = htons(port);              /* Local port */
	
	/* Bind to the local address */
	if (bind(ServerSocket, (struct sockaddr *) &ServAddr, sizeof(ServAddr)) < 0) {
	    printf("Can't bind to given port number. Trying a different one.\n"); 
	    port++;
	}
	else
	{ 
	  myself.change_port(port);
	  return 0;
	}  
	tries++;
    }
    
    return -1;
}

int NetworkCommWorkerCommunicator::receive()
{
	struct sockaddr_in cliaddr; /* Client address */
        socklen_t len = sizeof(cliaddr);
        char tmpbuffer[MAXINDSIZE];
        unsigned int recvMsgSize;
                /*receive UDP datagrams from client*/
	while(!cancel)
	{  
	    if ((recvMsgSize = recvfrom(ServerSocket,tmpbuffer,MAXINDSIZE,0,(struct sockaddr *)&cliaddr,&len)) < 0) {
		    printf("\nError recvfrom()\n");
		    return -1;
	    }
      
	    printf("    Received individual from %s:%d\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
	    tmpbuffer[recvMsgSize] = 0;
	    std::string buffer(tmpbuffer);
	    pthread_mutex_lock(&server_mutex);
	    data.push(buffer);
	    pthread_mutex_unlock(&server_mutex);
	}
	return 0;
}

int NetworkCommWorkerCommunicator::send(char* individual, CommWorker& destination)
{
   int sendSocket;
   sockaddr_in destaddr;

   if ((sendSocket = socket(AF_INET,SOCK_DGRAM,0)) < 0) {
        printf("Socket create problem."); return -1;
    }
    
    
    int sendbuff=35000;

    setsockopt(sendSocket, SOL_SOCKET, SO_SNDBUF, &sendbuff, sizeof(sendbuff));

    std::string complete_ind = myself.get_name() + "::" + individual;
	if( complete_ind.length() < (unsigned)sendbuff ) { 
		
	        destaddr.sin_family = AF_INET;
		destaddr.sin_addr.s_addr = inet_addr(destination.get_ip().c_str());
		destaddr.sin_port = htons(destination.get_port());
		int n_sent = sendto(sendSocket,complete_ind.c_str(),complete_ind.length(),0,(struct sockaddr *)&destaddr,sizeof(destaddr));
		
		//int n_sent = sendto(this->Socket,t,strlen(individual),0,(struct sockaddr *)&this->ServAddr,sizeof(this->ServAddr));
		if( n_sent < 0){
			printf("Size of the individual %d\n", (int)strlen(individual));
			perror("! Error while sending the message !");
			return -1;
		}
		else
		{  
		  if(debug)
		    printf("Individual sent to hostname: %s  ip: %s  port: %d\n", destination.get_name().c_str(),
			   destination.get_ip().c_str(),destination.get_port());
		}	   
	}
	else {fprintf(stderr,"Not sending individual with strlen(): %i, MAX msg size %i\n",(int)strlen(individual), sendbuff);}
	close(sendSocket);
	return 0;
}
