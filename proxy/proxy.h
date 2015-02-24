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
class Proxy {
private:
	static const int MAXBUFLEN =100;
	char recvBuf[MAXBUFLEN];
	char sendBuf[MAXBUFLEN];
	int recvLen;
	int sendLen;
	int port;
	int sockfd;
	int tunfd;
	char Eth0[16];
	int MaxNumRouter;
	int NumRouterConn;
	connectInfo_t *routerAddr;
	connectInfo_t *routerTail;

	/*tunnel related*/
	int tun_alloc(char *dev, int flags);

	/*eth0 ip get*/
	void get_eth0_ip();

	/*add router*/
	void proxy_add_routerAddr(sockaddr *sa);
public:
	Proxy();
	/*proxy setup*/
	void proxy_setup();

	/*router setup*/
	void proxy_routerReady();

	/*print information*/
	void proxy_info();
	void proxy_routerList();
};
