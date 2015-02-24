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
	new_proxy.proxy_setup();
	new_proxy.proxy_info();
	new_proxy.proxy_routerReady();
	//new_proxy.proxy_check_routerList();
	return EXIT_SUCCESS;
}				// ----------  end of function main  ----------
