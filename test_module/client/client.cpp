// =====================================================================================
//
//       Filename:  client.cpp
//
//    Description:  test client to check connection
//
//        Version:  1.0
//        Created:  02/12/2015 05:42:49 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Chen Xie (Neil), xiec@usc.edu
//   Organization:  USC
//
// =====================================================================================
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

using namespace std;

#define SERVERPORT "19121"    // the port users will be connecting to
#define MYPORT "19123"
int main(int argc, char *argv[])
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p,*myinfo;
	int rv;
	int numbytes;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo("192.168.201.2", SERVERPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	getaddrinfo("192.168.202.2", MYPORT,&hints,&myinfo);
	//loop through all the results and make a socket
	for(p = myinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
						p->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "talker: failed to bind socket\n");
		return 2;
	}
	bind(sockfd,p->ai_addr,p->ai_addrlen);
	char msg[100];
	memset(msg, 0, sizeof msg);
	sprintf(msg,"try to connect by client");
	if ((numbytes = sendto(sockfd, msg, strlen(msg), 0,
					servinfo->ai_addr, servinfo->ai_addrlen)) == -1) {
		perror("talker: sendto");
		exit(1);
	}

	freeaddrinfo(servinfo);

	printf("talker: sent %d bytes to %s\n", numbytes, argv[1]);
	/*check own info*/
	//struct sockaddr mySock;
	//socklen_t mySockLen = sizeof mySock;
	//getsockname(sockfd,&mySock,&mySockLen);
	close(sockfd);
	return 0;
}
