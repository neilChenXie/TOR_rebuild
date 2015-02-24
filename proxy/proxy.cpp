// =====================================================================================
//
//       Filename:  proxy.cpp
//
//    Description:  implementation of proxy class
//
//        Version:  1.0
//        Created:  02/11/2015 02:54:29 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Chen Xie (Neil), xiec@usc.edu
//   Organization:  USC
//
// =====================================================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if_tun.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <fcntl.h>
/*class define*/
#include "proxy.h"
using namespace std;

/****************************private method*******************************/
void Proxy::get_eth0_ip() {
	int fd;
	struct ifreq ifr;

	fd = socket(AF_INET, SOCK_DGRAM, 0);

	/* I want to get an IPv4 IP address */
	ifr.ifr_addr.sa_family = AF_INET;

	/* I want IP address attached to "eth0" */
	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);

	ioctl(fd, SIOCGIFADDR, &ifr);

	close(fd);

	/* display result */
	sprintf(Eth0,"%s", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
	printf("proxy ip:%s\n",Eth0);
}
/*
 *allocate tunnel fd which is used to read packet from tunnel
 * */
int Proxy::tun_alloc(char *dev, int flags) {
	struct ifreq ifr;
	int fd, err;
	char *clonedev = (char*)"/dev/net/tun";

	if( (fd = open(clonedev , O_RDWR)) < 0 ) 
	{
		perror("Opening /dev/net/tun");
		return fd;
	}

	memset(&ifr, 0, sizeof(ifr));

	ifr.ifr_flags = flags;

	if (*dev) 
	{
		strncpy(ifr.ifr_name, dev, IFNAMSIZ);
	}

	if( (err = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0 ) 
	{
		perror("ioctl(TUNSETIFF)");
		close(fd);
		return err;
	}

	strcpy(dev, ifr.ifr_name);
	return fd;
}
/****************************public method********************************/
Proxy::Proxy() 
{
	memset(recvBuf, 0, sizeof recvBuf);
	memset(sendBuf, 0, sizeof sendBuf);
	//sprintf(Eth0,"localhost");
	get_eth0_ip();
	port = 19121;
	MaxNumRouter = 4;
	NumRouterConn = 0;
	routerAddr = NULL;
	routerTail = NULL;
}

/*
 *create socket for proxy
 * */
void Proxy::proxy_setup() {
	struct addrinfo hints, *res;
	char setupPort[5];
	int rv;

	/*set setupPort*/
	sprintf(setupPort,"%d",port);

	/*type of socket*/
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;

	/*get socket creation information*/
	if((rv = getaddrinfo(Eth0, setupPort, &hints, &res)) == -1) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(rv));
		exit(1);
	}

	/*create socket*/
	if((rv = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
		perror("proxy: socket()");
		exit(1);
	}
	sockfd = rv;

	/*bind*/
	if((rv = bind(sockfd, res->ai_addr, res->ai_addrlen)) == -1) {
		close(sockfd);
		perror("proxy:bind()");
		exit(1);
	}
	freeaddrinfo(res);
	
	/*setup tunnel*/
    char tun_name[IFNAMSIZ];
    strcpy(tun_name, "tun1");
	tunfd = tun_alloc(tun_name, IFF_TUN | IFF_NO_PI);

	/*setup complete*/
	printf("proxy: socket setup complete\n");
}

void Proxy::proxy_routerReady() {
	/*receive packet from router*/
	while (NumRouterConn < MaxNumRouter) {
		struct sockaddr their_addr;
		socklen_t addr_len = sizeof their_addr;

		memset(recvBuf, 0, sizeof recvBuf);
		recvLen = recvfrom(sockfd, recvBuf, MAXBUFLEN - 1, 0, &their_addr, &addr_len);
		if(recvLen == -1) {
			perror("proxy_routerReady()");
			exit(1);
		}
		proxy_add_routerAddr(&their_addr);
		//proxy_check_routerList();
		recvBuf[recvLen] = '\0';
		NumRouterConn++;
	}
}

void Proxy::proxy_add_routerAddr(sockaddr *sa) {
	char s[INET6_ADDRSTRLEN];
	connectInfo_t *newCI = new connectInfo_t;

	/*initial new member*/
	memset(newCI, 0, sizeof *newCI);
	newCI->inAddr = ((struct sockaddr_in *)sa)->sin_addr;
	sprintf(newCI->charAddr,"%s",inet_ntop(sa->sa_family, &(newCI->inAddr), s, sizeof s));
	newCI->udp_port = ntohs(((struct sockaddr_in *)sa)->sin_port);
	newCI->next = NULL;

	/*print infomation*/
	printf("----------client connected---------\n");
	printf("Index: %d\n",NumRouterConn);
	printf("IP: %s\n", newCI->charAddr);
	printf("UDP port: %d\n", newCI->udp_port);
	printf("TCP port: %d\n", newCI->tcp_port);
	printf("-----------------------------------\n");

	/*add to link list*/
	if(routerAddr == NULL) {
		routerAddr = newCI;	
		routerTail = newCI;
	} else {
		routerTail->next = newCI;
		routerTail = newCI;
	}
}

void Proxy::proxy_routerList() {
	if(routerAddr == NULL) {
		printf("proxy: Router list is empty\n");
	} else {
		connectInfo_t *tmp = routerAddr;
		int count = 0;
		while(tmp != NULL) {
			printf("-----------------------------------\n");
			printf("Index: %d\n",count);
			printf("IP: %s\n", tmp->charAddr);
			printf("UDP port: %d\n", tmp->udp_port);
			printf("TCP port: %d\n", tmp->tcp_port);
			printf("-----------------------------------\n");
			tmp = tmp->next;
			count++;
		}
	}
}

void Proxy::proxy_info() {
	printf("-----------------------------------\n");
	printf("ip addr: %s\n", Eth0);
	printf("socket fd: %d\n",sockfd);
	printf("port: %d\n", port);
	printf("router num: %d\n",NumRouterConn);
	printf("-----------------------------------\n");
	printf("Router info:\n");
	proxy_routerList();
}
