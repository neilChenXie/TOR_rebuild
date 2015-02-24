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
#define	MAXMSGLEN 1024			// 
class Router {
	private:
		int router_index;
		char recvBuf[MAXMSGLEN];
		char sendBuf[MAXMSGLEN];
		int recvLen;
		int sendLen;
		int udpPort;
		char ethAddr[16];
		char Eth0[16];

		/*router info*/
		void router_proxyip();
		void router_myip();
	public:
		Router(int index);
		void router_setup();
		void router_notify_proxy();
		void send();
		/*print info*/
		void router_print_info();
};
