#ifndef _console_
#define _console_

#include "setup.h"
//#include <QT\qdatastream.h>
#include <QtNetwork/QtNetwork>
#include <QtCore/QSignalMapper>

#include <stdlib.h>
#include <iostream>
#include <set>
#include <string>
#include <vector>
#include <sstream>
using namespace std;

#pragma pack(4)

typedef struct  {
	char     source[32];
	uint16_t length;
	std::stringstream  body;
}  sConsoleMessage;
//QDataStream& operator << (QDataStream& s, const sConsoleMessage& x);
//QDataStream& operator >> (QDataStream& s, sConsoleMessage& x);
stringstream& operator << (stringstream& s, const sConsoleMessage& x);
stringstream& operator >> (stringstream& s, sConsoleMessage& x);
ostream& operator<< (ostream& s, const sConsoleMessage& x);

typedef struct  {
	uint32_t numMsg;
	sConsoleMessage Msgs[MAX_NUM_CONSOLE_MESSAGES];
} sConsole;
//QDataStream& operator << (QDataStream& s, const sConsole& x);
//QDataStream& operator >> (QDataStream& s, sConsole& x);
stringstream& operator << (stringstream& s, const sConsole& x);
stringstream& operator >> (stringstream& s, sConsole& x);
ostream& operator<< (ostream& s, const sConsole& x);




#endif
