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
	char Eth0[16];
	int MaxNumRouter;
	int NumRouterConn;
	connectInfo_t *routerAddr;
	connectInfo_t *routerTail;

	void proxy_add_routerAddr(sockaddr *sa);
public:
	Proxy();
	void proxy_setup();
	void proxy_routerReady();
	void proxy_info();
	void proxy_check_routerList();
};
