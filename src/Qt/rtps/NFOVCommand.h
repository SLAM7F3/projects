#ifndef _nfov_command_
#define _nfov_command_

#include "setup.h"
#include "UTMdef.h"
//#include <QT\qdatastream.h>
#include <QtNetwork/QtNetwork>
#include <QtCore/QSignalMapper>

#include <stdio.h>
#include <iostream>
#include <sstream>
#include <string>
using namespace std;

#pragma pack(4)

// NFOV Command
typedef struct {
	uint32_t		currentNFOVCommandID;
	bool			enable;
	sUTMCoord		utmCoord;
	uint32_t		imageUpdatePriority;
	float			imagePeriod;
	uint32_t		jpegQuality;
} sNFOVCommand;
QDataStream& operator << (QDataStream& s, const sNFOVCommand& x);
QDataStream& operator >> (QDataStream& s, sNFOVCommand& x);
stringstream& operator << (stringstream& s, const sNFOVCommand& x);
stringstream& operator >> (stringstream& s, sNFOVCommand& x);
ostream& operator<< (ostream& s, const sNFOVCommand& x);

#endif
