// =====================================================================================
//
//       Filename:  router.cpp
//
//    Description:  implementation of TOR router
//
//        Version:  1.0
//        Created:  02/24/2015 02:21:10 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Chen Xie (Neil), xiec@usc.edu
//   Organization:  USC
//
// =====================================================================================
/*class define*/
#include "router.h"
using namespace std;
/***************************private method******************************/
struct in_addr Router::get_eth_by_name(const char *ethn) {
	int fd;
	struct ifreq ifr;

	fd = socket(AF_INET, SOCK_DGRAM, 0);

	/* I want to get an IPv4 IP address */
	ifr.ifr_addr.sa_family = AF_INET;

	/* I want IP address attached to "eth0" */
	strncpy(ifr.ifr_name, ethn, IFNAMSIZ-1);

	ioctl(fd, SIOCGIFADDR, &ifr);

	close(fd);

	/* display result */
	return ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr;
}

void Router::get_all_ethn() {
	int i = 0;
	char ethn[5];
	while (i < MAXETHADDR) {
		memset(ethn,'0',sizeof ethn);
		sprintf(ethn,"eth%d",i);
		ethAddr[i] = get_eth_by_name(ethn);
		i++;
	}
}
/************************public method*********************************/
Router::Router(int index) {
	router_index = index;
	udpPort = 19121;
	tcpPort = 19123;
	get_all_ethn();
}

void Router::router_setup() {
	udp_sock_setup(2*router_index);
	tcp_sock_setup(2*router_index);
	get_proxy_info();

	/*setup complete*/
	printf("router %d: socket setup complete\n",router_index);
}

void Router::udp_sock_setup(int ethIndex) {
	struct addrinfo hints, *res;
	char setupPort[5];
	int rv;

	/*set setupPort*/
	sprintf(setupPort,"%d",udpPort);

	/*type of socket*/
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;

	/*get socket creation information*/
	if((rv = getaddrinfo(inet_ntoa(ethAddr[ethIndex]), setupPort, &hints, &res)) == -1) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(rv));
		exit(1);
	}

	/*create socket*/
	if((rv = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
		perror("proxy: socket()");
		exit(1);
	}

	/*__udpfd__*/
	udpfd = rv;

	/*bind*/
	if((rv = bind(udpfd, res->ai_addr, res->ai_addrlen)) == -1) {
		close(udpfd);
		perror("proxy:bind()");
		exit(1);
	}
	freeaddrinfo(res);
}

void Router::tcp_sock_setup(int ethIndex) {
	struct addrinfo hints, *res;
	char setupPort[5];
	int rv;

	/*set setupPort*/
	sprintf(setupPort,"%d",tcpPort);

	/*type of socket*/
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	/*get socket creation information*/
	if((rv = getaddrinfo(inet_ntoa(ethAddr[ethIndex]), setupPort, &hints, &res)) == -1) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(rv));
		exit(1);
	}

	/*create socket*/
	if((rv = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
		perror("proxy: socket()");
		exit(1);
	}

	/*__udpfd__*/
	tcpfd = rv;

	/*bind*/
	if((rv = bind(tcpfd, res->ai_addr, res->ai_addrlen)) == -1) {
		close(udpfd);
		perror("proxy:bind()");
		exit(1);
	}
	freeaddrinfo(res);
}
void Router::get_proxy_info() {
	struct addrinfo hints, *res;
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	
	rv = getaddrinfo(inet_ntoa(ethAddr[1]),"19121",&hints,&res);

	if(rv == -1) {
		fprintf(stderr, "router:getaddrinfo %s\n", gai_strerror(rv));
		exit(1);
	}

	memcpy(&proxySock[0],res->ai_addr,sizeof proxySock[0]);
}
/************************communication**********************************/
struct sockaddr Router::router_udp_recv() {
	struct sockaddr their_sock;
	socklen_t sockLen = sizeof their_sock;
	
	memset(recvBuf,0,sizeof recvBuf);
	recvLen = recvfrom(udpfd,recvBuf, MAXBUFLEN - 1, 0, &their_sock,&sockLen);
	if(recvLen == -1) {
		perror("router:udp recv\n");
		exit(1);
	}

	printf("___udp receive from____\n");
	sockaddr_info(&their_sock);
	recvBuf[recvLen] = '\0';
	return their_sock;
}

void Router::router_udp_send(struct sockaddr *dstSock) {
	int rv;
	struct sockaddr tmp;
	socklen_t sockLen = sizeof tmp;
	rv = sendto(udpfd,sendBuf,sendLen,0, dstSock, sockLen);
	if(rv == -1) {
		perror("router:udp send\n");
		exit(1);
	}
}
/****************************router info******************************/
void Router::router_info() {
	printf("-----------Proxy info-------------\n");
	ethn_info();
	sock_info();
	proxy_info();
	printf("-----------------------------------\n");
}
void Router::sockaddr_info(struct sockaddr *sock) {
	if(sock->sa_family == AF_INET) {
		struct sockaddr_in *tmpSock = (struct sockaddr_in *) sock;
		printf("%s:%u\n", inet_ntoa(tmpSock->sin_addr),ntohs(tmpSock->sin_port));
	} else {
		struct sockaddr_in6 *tmpSock = (struct sockaddr_in6 *)sock;
		char s[INET6_ADDRSTRLEN];
		inet_ntop(AF_INET6,&(tmpSock->sin6_addr),s,INET6_ADDRSTRLEN);
		printf("%s:%u\n",s,ntohs(tmpSock->sin6_port));
	}
}
void Router::sock_info() {
	printf("____sock info___\n");
	printf("UDP sockfd: %d port: %d\n", udpfd, udpPort);
	printf("TCP sockfd: %d port: %d\n", tcpfd, tcpPort);
	printf("RAW sockfd: %d port: %d\n", rawfd, rawPort);
}
void Router::ethn_info() {
	int i = 0;
	printf("____eth info___\n");
	while(i<MAXETHADDR) {
		printf("eth%d: %s\n",i,inet_ntoa(ethAddr[i]));
		i++;
	}
}
void Router::proxy_info() {
	int i = 0;
	while (i < MAXPROXY) {
		printf("proxy %d: ",i+1);
		sockaddr_info((struct sockaddr*)&proxySock[i]);
		i++;
	}
}
/******************specific task related method************************/
void Router::router_TOR_run() {
	router_setup();
	router_info();
	send_hello();
}

void Router::send_hello() {
	memset(sendBuf, 0, sizeof sendBuf);
	sprintf(sendBuf,"hello");
	sendLen = strlen(sendBuf);
	router_udp_send(&proxySock[0]);
}
