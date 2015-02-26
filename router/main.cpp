// =====================================================================================
//
//       Filename:  main.cpp
//
//    Description:  test main for router
//
//        Version:  1.0
//        Created:  02/15/2015 07:41:44 PM
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
#include "router.h"
using namespace std;

int main ( int argc, char *argv[] )
{
	Router my_router(1);
	//my_router.router_setup();
	my_router.router_TOR_run();
	return EXIT_SUCCESS;
}				// ----------  end of function main  ----------
