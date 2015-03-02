// =====================================================================================
//
//       Filename:  router.h
//
//    Description:  router class for TOR project
//
//        Version:  1.0
//        Created:  02/15/2015 07:43:24 PM
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
#include <assert.h>
using namespace std;
class Router {
	private:
		static const int MAXBUFLEN = 1024;			// 
		static const int MAXETHADDR = 8;
		static const int MAXPROXY = 1;
		/*information*/
		int router_index;
		int udpPort;
		int tcpPort;
		int rawPort;
		int udpfd;
		int tcpfd;
		int rawfd;
		struct in_addr ethAddr[MAXETHADDR]; //VM config related
		struct sockaddr proxySock[MAXPROXY];
		/*communication*/
		char recvBuf[MAXBUFLEN];
		char sendBuf[MAXBUFLEN];
		int recvLen;
		int sendLen;

		/*eth address get*/
		struct in_addr get_eth_by_name(const char *ethn);
		void get_all_ethn();

		void router_myip();
	public:
		Router(int index);
		/*rotuer setup*/
		void router_setup();
		void udp_sock_setup(int ethIndex);
		void tcp_sock_setup(int ethIndex);
		void get_proxy_info();
		/*communication*/
		void create_sendBuf(char* data, int length);
		struct sockaddr router_udp_recv();
		void router_udp_send(struct sockaddr *dstSock);

		/*print info*/
		void router_info();
		void sock_info();
		void ethn_info();
		void proxy_info();
		void sockaddr_info(struct sockaddr* sock);
		/*packet information*/
		void print_IP_packet(char *ipPkt);
		void print_ICMP_packet(char *payload);
		void print_TCP_packet(char *payload);
		void print_UDP_packet(char *payload);
		void print_binary(char *data, int length);
		/*specific task related function*/
		void router_TOR_run();
		//void send_hello();
};
