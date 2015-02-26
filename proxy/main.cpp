// =====================================================================================
//
//       Filename:  main.cpp
//
//    Description:  test proxy class
//
//        Version:  1.0
//        Created:  02/12/2015 12:08:41 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Chen Xie (Neil), xiec@usc.edu
//   Organization:  USC
//
// =====================================================================================

#include <iostream>
#include <cstdlib>
#include <string>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "proxy.h"
using namespace std;

int main ( int argc, char *argv[] )
{
	Proxy new_proxy;
	//new_proxy.router_ready_check();
	new_proxy.proxy_TOR_run();
	return EXIT_SUCCESS;
}				// ----------  end of function main  ----------
