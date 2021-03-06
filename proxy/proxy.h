// =====================================================================================
//
//       Filename:  proxy.h
//
//    Description:  proxy defination
//
//        Version:  1.0
//        Created:  02/11/2015 02:33:21 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Chen Xie (Neil), xiec@usc.edu
//   Organization:  USC
//
// =====================================================================================
#include "../protocol/protocol.h"
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
#include <netinet/ip_icmp.h>
#include <assert.h>
using namespace std;

class Proxy {
private:
	static const int MAXBUFLEN = 100;
	static const int MAXETHADDR = 8;
	static const int MAXROUTER = 1;
	/*characteristic*/
	uint16_t udpport;
	uint16_t tcpport;
	uint16_t rawport;
	int tunfd;
	int udpfd;
	int tcpfd;
	int rawfd;
	struct in_addr ethAddr[MAXETHADDR]; //VM config related
	/*router info*/
	struct sockaddr_in routerSock[MAXROUTER];
	struct sockaddr_in6 routerSock6[MAXROUTER];
	/*communication*/
	char recvBuf[MAXBUFLEN];
	char sendBuf[MAXBUFLEN];
	int recvLen;
	int sendLen;

	/*eth address get*/
	struct in_addr get_eth_by_name(const char *ethn);
	void get_all_ethn();
	/*tunnel related*/
	int tun_alloc(char *dev, int flags);

public:
	Proxy();
	/*proxy setup*/
	void proxy_setup();
	void udp_sock_setup(int ethIndex);
	void tcp_sock_setup(int ethIndex);
	void tunnel_setup(char *tunName);

	/*communication*/
	void create_sendBuf(char*data,int length);
	void clear_recvBuf();
	struct sockaddr proxy_udp_recv();
	void proxy_udp_send(struct sockaddr *dstSock);
	void proxy_tun_recv();
	void proxy_tun_send();
	int proxy_tcp_connect(int ethIndex, int port);
	int proxy_select_udp_tun();

	/*revise packet header*/
	uint16_t ipChecksum(const void *pkt, size_t headLen);
	void IP_header_revise(struct ip* pkt, struct in_addr dstIP, struct in_addr recIP);

	/*proxy information*/
	void proxy_info();
	void sock_info();
	void ethn_info();
	void tun_info();
	void router_info();
	void sockaddr_info(struct sockaddr* sock);
	/*packet information*/
	void print_IP_packet(char *ipPkt);
	void print_ICMP_packet(char *payload);
	void print_TCP_packet(char *payload);
	void print_UDP_packet(char *payload);
	void print_binary(char *data, int length);
	//void proxy_routerList();
	/*specific task related function*/
	void proxy_TOR_run();
	void router_ready_check();
};
