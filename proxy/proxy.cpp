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
/*class define*/
#include "proxy.h"
using namespace std;

/***************************private method*******************************/
struct in_addr Proxy::get_eth_by_name(const char *ethn) {
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
void Proxy::get_all_ethn() {
	int i = 0;
	char ethn[5];
	while (i < MAXETHADDR) {
		memset(ethn,'0',sizeof ethn);
		sprintf(ethn,"eth%d",i);
		ethAddr[i] = get_eth_by_name(ethn);
		i++;
	}
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
/***************************public method********************************/
Proxy::Proxy() 
{
	memset(recvBuf, 0, sizeof recvBuf);
	memset(sendBuf, 0, sizeof sendBuf);
	//sprintf(Eth0,"localhost");
	get_all_ethn();
	udpport = 19121;
	tcpport = 19123;
	rawport = 0;
	udpfd = 0;
	tcpfd = 0;
	rawfd = 0;
}

/*
 *create socket for proxy
 * */
void Proxy::udp_sock_setup(int ethIndex) {
	struct addrinfo hints, *res;
	char setupPort[5];
	int rv;

	/*set setupPort*/
	sprintf(setupPort,"%d",udpport);

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

void Proxy::tcp_sock_setup(int ethIndex) {
	struct addrinfo hints, *res;
	char setupPort[5];
	int rv;

	/*set setupPort*/
	sprintf(setupPort,"%d",tcpport);

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

void Proxy::tunnel_setup(char *tunName) {
	/*setup tunnel*/
    char tun_name[IFNAMSIZ];
    strcpy(tun_name, tunName);
	tunfd = tun_alloc(tun_name, IFF_TUN | IFF_NO_PI);
}

void Proxy::proxy_setup() {
	char tunN[5];
	udp_sock_setup(1);
	tcp_sock_setup(1);
	memset(tunN,0,sizeof tunN);
	sprintf(tunN,"tun1");
	tunnel_setup(tunN);

	/*setup complete*/
	printf("proxy: socket setup complete\n");
}

/*******************communication*****************************/
void Proxy::create_sendBuf(char *data, int length) {
	assert(data != NULL);
	memset(sendBuf,0,sizeof sendBuf);
	sendLen = length;
	memcpy(sendBuf,data,length);
}
void Proxy::clear_recvBuf() {
	memset(recvBuf,0,sizeof recvBuf);
}
struct sockaddr Proxy::proxy_udp_recv() {
	/*receive packet from router*/
	struct sockaddr their_addr;
	socklen_t addr_len = sizeof their_addr;

	memset(recvBuf, 0, sizeof recvBuf);
	recvLen = recvfrom(udpfd, recvBuf, MAXBUFLEN - 1, 0, &their_addr, &addr_len);
	if(recvLen == -1) {
		perror("proxy_routerReady()");
		exit(1);
	}

	printf("___udp receive from____\n");
	sockaddr_info(&their_addr);
	//proxy_add_routerAddr(&their_addr);
	//proxy_check_routerList();
	recvBuf[recvLen] = '\0';
	return their_addr;
}

void Proxy::proxy_udp_send(struct sockaddr* dstSock) {
	int rv;
	sockaddr tmp;
	socklen_t sockLen = sizeof tmp;
	rv = sendto(udpfd, sendBuf, sendLen, 0, dstSock, sockLen);
	if(rv == -1) {
		perror("proxy:UDP sendto\n");
		exit(1);
	}
}
void Proxy::proxy_tun_recv() {
	memset(recvBuf, 0, sizeof recvBuf);
	recvLen = read(tunfd,recvBuf,sizeof recvBuf);
}

int Proxy::proxy_tcp_connect(int ethIndex, int port) {
	struct addrinfo hints, *res;
	char setport[6];
	int rv;

	memset(&hints, 0, sizeof hints);
	memset(setport,0,sizeof setport);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	sprintf(setport,"%d",port);
	rv = getaddrinfo(inet_ntoa(ethAddr[ethIndex]),setport,&hints,&res);
	if(rv != 0) {
		fprintf(stderr, "getaddrinfo:%s\n",gai_strerror(rv));
		return -1;
	}
	rv = connect(tcpfd,res->ai_addr,res->ai_addrlen);
	if(rv == -1) {
		perror("proxy:connect");
		return -1;
	}
	return 0;
}

int Proxy::proxy_select_udp_tun() {
	fd_set readfd;
	int maxfd;

	FD_ZERO(&readfd);
	FD_SET(udpfd,&readfd);
	FD_SET(tunfd,&readfd);
	if(udpfd > tunfd) {
		maxfd = udpfd;
	} else {
		maxfd = tunfd;
	}

	/*wait for traffic*/
	select(maxfd + 1, &readfd, NULL, NULL, NULL); //never timeout

	if(FD_ISSET(udpfd,&readfd)) {
		/*for udp*/
		proxy_udp_recv();
		printf("proxy:recv %d bytes from UDP\n",recvLen);
		return 1;
	}
	if(FD_ISSET(tunfd,&readfd)) {
		/*for tunnel*/
		proxy_tun_recv();
		printf("proxy:recv %d bytes from tunnel\n",recvLen);
		return 2;
	}
	return 0;
}
/*************************moniter****************************************/
//void Proxy::proxy_routerList() {
//	if(routerAddr == NULL) {
//		printf("proxy: Router list is empty\n");
//	} else {
//		connectInfo_t *tmp = routerAddr;
//		int count = 0;
//		while(tmp != NULL) {
//			printf("-----------------------------------\n");
//			printf("Index: %d\n",count);
//			printf("IP: %s\n", tmp->charAddr);
//			printf("UDP port: %d\n", tmp->udp_port);
//			printf("TCP port: %d\n", tmp->tcp_port);
//			printf("-----------------------------------\n");
//			tmp = tmp->next;
//			count++;
//		}
//	}
//}
//
void Proxy::sockaddr_info(struct sockaddr *sock) {
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
void Proxy::sock_info() {
	printf("____sock info___\n");
	printf("UDP sockfd: %d port: %d\n", udpfd, udpport);
	printf("TCP sockfd: %d port: %d\n", tcpfd, tcpport);
	printf("RAW sockfd: %d port: %d\n", rawfd, rawport);
}
void Proxy::ethn_info() {
	int i = 0;
	printf("____eth info___\n");
	while(i<MAXETHADDR) {
		printf("eth%d: %s\n",i,inet_ntoa(ethAddr[i]));
		i++;
	}
}
void Proxy::tun_info() {
	printf("__tunnel info___\n");
	printf("tunnel fd: %d\n",tunfd);
}
void Proxy::proxy_info() {
	printf("-----------Proxy info-------------\n");
	ethn_info();
	sock_info();
	tun_info();
	printf("-----------------------------------\n");
}
void Proxy::router_info() {
	int i = 0;
	while (i < MAXROUTER) {
		printf("router%d: ",i+1);
		sockaddr_info((struct sockaddr*)&routerSock[i]);
		i++;
	}
}
/*********************packet information*********************/
void Proxy::print_IP_packet(char *ipPkt) {
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
	printf("TTL:\t\t%u\n",pktDetail->ip_ttl);
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
void Proxy::print_ICMP_packet(char *payload) {
	struct icmp *icmph = (struct icmp *)payload;
	printf("____ICMP packet______\n");
	printf("icmp: type %d code %d\n",icmph->icmp_type,icmph->icmp_code);
}
void Proxy::print_TCP_packet(char *payload) {
	printf("____TCP packet______\n");
}
void Proxy::print_UDP_packet(char *payload) {
	printf("____UDP packet______\n");
}
void Proxy::print_binary(char *data, int length) {
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
/***********************task method**************************/
void Proxy::proxy_TOR_run() {
	proxy_setup();
	proxy_info();
	router_ready_check();
	router_info();
	int pktInfo;
	while(1) {	
		pktInfo = proxy_select_udp_tun();
		if(pktInfo == 1) {
			/*from UDP*/
			printf("%s\n",recvBuf);
		}
		if(pktInfo == 2) {
			/*from tunnel*/
			print_IP_packet(recvBuf);
			/*prepare sendBuf*/
			create_sendBuf(recvBuf,recvLen);
			proxy_udp_send((struct sockaddr *)&routerSock[0]);
			clear_recvBuf();
		}
	}
}
void Proxy::router_ready_check() {
	int i = 0;
	int ii = 0;
	struct sockaddr tmp;
	while ((i+ii) < MAXROUTER) {
		tmp = proxy_udp_recv();
		if(tmp.sa_family == AF_INET) {
			memcpy(&routerSock[i],&tmp,sizeof tmp);
			//printf("__check after storage_\n");
			sockaddr_info((struct sockaddr*)&routerSock[i]);
			i++;
		} else {
			memcpy(&routerSock6[i],&tmp,sizeof tmp);
			//printf("__check after storage_\n");
			sockaddr_info((struct sockaddr*)&routerSock6[i]);
			ii++;
		}
		printf("received length %d: %s\n",recvLen,recvBuf);
	}
	printf("all router are ready\n");
}

