// =====================================================================================
//
//       Filename:  protocol.cpp
//
//    Description:  functions for network protocol
//
//        Version:  1.0
//        Created:  03/01/2015 10:54:29 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Chen Xie (Neil), xiec@usc.edu
//   Organization:  USC
//
// =====================================================================================
#include "protocol.h"

void print_IP_packet(char *ipPkt) {
	struct ip *pktDetail;
	pktDetail = (struct ip*)ipPkt;
	char ipDst[20],ipSrc[20];
	int headLen = pktDetail->ip_hl << 2;
	printf("__________IP pakcet___________");
	printf("ip version:\t%u\n",pktDetail->ip_v);	
	printf("header Length:\t%d\n",headLen);
	printf("type of service:\t%u\n",pktDetail->ip_tos);
	printf("total length:\t%d\n",ntohs(pktDetail->ip_len));
	printf("identifier:\t%d\n",ntohs(pktDetail->ip_id));
	printf("flags:");
	printf("TTL:\t%u\n",pktDetail->ip_ttl);
	printf("protocol:\t%u\n",pktDetail->ip_p);
	printf("ip dst:\t%s\n",inet_ntop(AF_INET,(void*)&pktDetail->ip_dst,ipDst,16));
	printf("ip src:\t%s\n",inet_ntop(AF_INET,(void*)&pktDetail->ip_src,ipSrc,16));
	printf("______________________________");
}
