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
#define	DEBUG 1			// 
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
	raw_icmp_sock_setup(2*router_index+1);
	udp_sock_setup(2*router_index);
	tcp_sock_setup(2*router_index);
	get_proxy_info();

	/*setup complete*/
	printf("router %d: socket setup complete\n",router_index);
}

void Router::raw_icmp_sock_setup(int ethIndex) {
	struct addrinfo  *res;
	int rv;

	rv = getaddrinfo(inet_ntoa(ethAddr[ethIndex]),NULL,NULL,&res);
	if(rv == -1) {
		fprintf(stderr, "getaddrinfo error: %s\n",gai_strerror(rv));
		exit(1);
	}
	rawICMPfd = socket(AF_INET,SOCK_RAW,IPPROTO_ICMP);
	if(rawICMPfd == -1) {
		perror("router:raw_socket\n");
		exit(1);
	}
	rv = bind(rawICMPfd, res->ai_addr, res->ai_addrlen);
	if(rv == -1) {
		perror("router:raw_bind\n");
		exit(0);
	}
	freeaddrinfo(res);
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
void Router::create_sendBuf(char* data, int length) {
	assert(data!=NULL);
	memset(sendBuf, 0, sizeof sendBuf);
	memcpy(sendBuf,data,length);
	sendLen = length;
}
void Router::create_sendBuf_with_IP_payload(const struct ip* data, int len) {
	int headLen = data->ip_hl << 2;
	int payloadLen = len - headLen;
	char *payload = (char*) data + headLen;

	memset(sendBuf, 0, sizeof sendBuf);
	memcpy(sendBuf, (char*)payload, payloadLen);
	if(DEBUG == 1) {
		printf("create_sendBuf_with_IP_payload():sendLen %d Bytes\n",payloadLen);
	}
	sendLen = payloadLen;
}
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

void Router::router_raw_icmp_send(struct in_addr ip_dst) {
	struct msghdr msg;
	struct sockaddr_in tmp;
	struct iovec iov;
	int rv;

	tmp.sin_family = AF_INET;
	tmp.sin_addr = ip_dst;
	tmp.sin_port = htons(0);

	msg.msg_name = &tmp;
	msg.msg_namelen = sizeof tmp;

	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	msg.msg_iov->iov_base = sendBuf;
	msg.msg_iov->iov_len = sendLen;

	msg.msg_control = 0;
	msg.msg_controllen = 0;
	msg.msg_flags = 0;

	rv = sendmsg(rawICMPfd, &msg, 0);
	if(rv == -1) {
		fprintf(stderr, "router:raw icmp send error\n");
		exit(1);
	}
	printf("router: raw_ icmp_send %d Bytes\n",rv);
}

void Router::router_raw_icmp_recv() {
	struct sockaddr their_addr;
	socklen_t addrLen = sizeof their_addr;

	memset(recvBuf, 0, sizeof recvBuf);
	recvLen = recvfrom(rawICMPfd, recvBuf, MAXBUFLEN-1, 0,(struct sockaddr*)&their_addr, &addrLen);
	if(recvLen == -1) {
		fprintf(stderr, "router: raw_icmp_recv error\n");
		exit(1);
	}
	printf("router: raw_icmp_recv %d Bytes\n",recvLen);
}
/**************************revise packet header***********************/
uint16_t Router::ipChecksum(const void *pkt, size_t headLen) {
	unsigned long sum = 0;
	uint16_t *ipl;
	
	ipl = (uint16_t *)pkt;

	while(headLen > 1) {
		sum += *ipl;
		ipl++;
		if(sum & 0x80000000)
			sum = (sum & 0xFFFF) + (sum >> 16);
		headLen -=2;
	}
	while(sum >> 16) {
		sum = (sum & 0xFFFF) + (sum >> 16);
	}
	return ~sum;
}
void Router::IP_header_revise(struct ip* pkt, struct in_addr dstIP, struct in_addr srcIP) {
	/*change addr*/
	pkt->ip_dst = dstIP;
	pkt->ip_src = srcIP;
	/*recalculate checksum*/
	memset(&(pkt->ip_sum), 0, sizeof(pkt->ip_sum));
	pkt->ip_sum = ipChecksum((const void *)pkt,pkt->ip_hl<<2);
}
/****************************router info******************************/
void Router::router_info() {
	printf("-----------Router info-------------\n");
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
	printf("RAW ICMP sockfd: %d\n", rawICMPfd);
	printf("RAW TCP sockfd: %d\n", rawTCPfd);
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
/*********************packet information*********************/
void Router::print_IP_packet(const char *ipPkt) {
	struct ip *pktDetail;
	pktDetail = (struct ip*)ipPkt;
	char ipDst[20],ipSrc[20];
	int headLen = pktDetail->ip_hl << 2;
	int totalLen = ntohs(pktDetail->ip_len);
	int payloadLen = totalLen - headLen;
	char *payload =(char *)pktDetail + headLen;
	printf("________IP packet_______\n");
	printf("ip version:\t%u\n",pktDetail->ip_v);	
	printf("header Length:\t%d\n",headLen);
	printf("type of service:%u\n",pktDetail->ip_tos);
	printf("total length:\t%d\n",totalLen);
	printf("identifier:\t%d\n",ntohs(pktDetail->ip_id));
	//printf("flags:");
	printf("TTL:\t%u\n",pktDetail->ip_ttl);
	printf("protocol:\t%u\n",pktDetail->ip_p);
	printf("destination IP:\t%s\n",inet_ntop(AF_INET,(void*)&pktDetail->ip_dst,ipDst,16));
	printf("source IP:\t%s\n",inet_ntop(AF_INET,(void*)&pktDetail->ip_src,ipSrc,16));
	switch(pktDetail->ip_p) {
		case IPPROTO_ICMP:
			print_ICMP_packet(payload);
			break;
		case IPPROTO_TCP:
			print_TCP_packet(payload);
			break;
		case IPPROTO_UDP:
			print_UDP_packet(payload);
			break;
		default:
			print_binary(payload,payloadLen);
	}
	printf("_________________________\n");
}
void Router::print_ICMP_packet(char *payload) {
	struct icmp *icmph = (struct icmp *)payload;
	printf("____ICMP packet______\n");
	printf("icmp: type %d code %d\n",icmph->icmp_type,icmph->icmp_code);
}
void Router::print_TCP_packet(char *payload) {
	printf("____TCP packet______\n");
}
void Router::print_UDP_packet(char *payload) {
	printf("____UDP packet______\n");
}
void Router::print_binary(char *data, int length) {
	printf("____Binary data_______\n");
	int i = 0;
	uint8_t *toPr = (uint8_t *)data;
	printf("data:");
	while (i < length) {
		toPr += i;
		printf("%02X", *toPr);
		i++;
	}
	printf("\n");
}
/******************specific task related method************************/
void Router::router_TOR_run() {
	router_setup();
	router_info();
	char msg[6] = "hello";
	create_sendBuf(msg,6);
	router_udp_send(&proxySock[0]);
	int i = 0;
	while (i < 1) {
		router_udp_recv();
		print_IP_packet(recvBuf);
		create_sendBuf_with_IP_payload((const struct ip*)recvBuf,recvLen);
		router_raw_icmp_send(((struct ip*)recvBuf)->ip_dst);
		router_raw_icmp_recv();
		print_IP_packet(recvBuf);
		/*send to proxy*/
		create_sendBuf(recvBuf,recvLen);
		router_udp_send(&proxySock[0]);
		i++;
	}
}

