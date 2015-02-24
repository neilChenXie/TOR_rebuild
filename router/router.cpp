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
#include "router.h"
using namespace std;
/***************************private method********************************/
void Router::router_proxyip() {
	int fd;
	struct ifreq ifr;

	fd = socket(AF_INET, SOCK_DGRAM, 0);

	/* I want to get an IPv4 IP address */
	ifr.ifr_addr.sa_family = AF_INET;

	/* I want IP address attached to "eth0" */
	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);

	ioctl(fd, SIOCGIFADDR, &ifr);

	close(fd);

	/*proxy infomation*/
	sprintf(Eth0,"%s", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
}
void Router::router_myip() {
	int fd;
	struct ifreq ifr;
	fd = socket(AF_INET, SOCK_DGRAM, 0);

	/* I want to get an IPv4 IP address */
	ifr.ifr_addr.sa_family = AF_INET;

	/* I want IP address attached to "eth0" */
	char ethIndex[5];
	memset(ethIndex,0,sizeof ethIndex);
	sprintf(ethIndex,"eth%d",router_index);
	strncpy(ifr.ifr_name, ethIndex, IFNAMSIZ-1);
	//strncpy(ifr.ifr_name, "eth1", IFNAMSIZ-1);
	

	ioctl(fd, SIOCGIFADDR, &ifr);
	sprintf(ethAddr,"%s", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
}
/***************************public method*********************************/
Router::Router(int index) {
	router_index = index;
	udpPort = 19123;
	router_proxyip();
	router_myip();
}

void router_setup() {

}
void Router::router_print_info() {
	printf("-----------------\n");
	printf("Eth0:%s\n",Eth0);
	printf("MyEth:%s\n",ethAddr);
	printf("-----------------\n");
}
