#ifndef _roi_command_
#define _roi_command_

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

// ROI Command
typedef struct  {
	uint32_t		regionID;
	bool			enable;
	sUTMCoord		utmCoord;
	uint32_t		trackID;
	uint32_t		trackUpdatePriority;
	uint32_t		imageUpdatePriority;
	float			imagePeriod;
	uint32_t		jpegQuality;
	uint32_t		numChips;
} sROICommand;
QDataStream& operator << (QDataStream& s, const sROICommand& x);
QDataStream& operator >> (QDataStream& s, sROICommand& x);
stringstream& operator << (stringstream& s, const sROICommand& x);
stringstream& operator >> (stringstream& s, sROICommand& x);
ostream& operator<< (ostream& s, const sROICommand& x);

#endif
