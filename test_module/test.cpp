// =====================================================================================
//
//       Filename:  test.cpp
//
//    Description:  test
//
//        Version:  1.0
//        Created:  02/11/2015 11:34:59 PM
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
using namespace std;
typedef struct aa{
	int a;
	char b[100];
}aa_t;
int main ( int argc, char *argv[] )
{
	aa_t *a = new aa_t;
	cout<<sizeof *a<<endl;
	memset(a, 0, sizeof *a);
	return EXIT_SUCCESS;
}				// ----------  end of function main  ----------
