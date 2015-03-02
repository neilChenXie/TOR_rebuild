// =====================================================================================
//
//       Filename:  protocol.h
//
//    Description:  include packet format and so on
//
//        Version:  1.0
//        Created:  02/11/2015 02:36:12 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Chen Xie (Neil), xiec@usc.edu
//   Organization:  USC
//
// =====================================================================================
#ifndef MINITORHEADER
#define MINITORHEADER
//cell length
#define TOTALLEN 		512
#define CONTROLMSGLEN 	509
#define RELAYMSGLEN 	498
//control msg related macro
#define PADDING 0x21
#define CREATE 	0x22
#define	CREATED 0x23			// 
#define	DESTROY 0x24			// 
//relay msg related macro
#define	RELAY 		0x30			// 
#define	RDATA 		0x31			// 
#define	RBEGIN 		0x32			// 
#define	REND		0x33			// 
#define	RTEARDOWN	0x34			// 
#define	RCONNECTED	0x35			// 
#define	REXTEND		0x36			// 
#define	REXTENDED	0x37			// 
#define	RTRUNCATE	0x38			// 
#define	RTRUNCATED	0x39			// 
#define	SENDME		0x40			// 
//head file for packet definition
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
using namespace std;

typedef struct control_pkt {
	uint16_t CircID;
	uint8_t  CMD;	
	uint8_t  DATA[CONTROLMSGLEN];
}control_cell_t;
typedef struct relay_pkt {
	uint16_t CircID;
	uint8_t	 Relay;
	uint16_t StreamID;
	uint8_t  Digest[6];
	uint16_t Len;
	uint8_t  CMD;
	uint8_t  DATA[RELAYMSGLEN];
}relay_cell_t;

typedef struct connectInfo {
	struct in_addr inAddr;//main index
	char charAddr[INET6_ADDRSTRLEN];
	uint16_t udp_port;
	uint16_t tcp_port;
	connectInfo *next;
}connectInfo_t;

/*functions about printf packet information*/
void print_IP_packet(char *ipPkt); 

#endif
