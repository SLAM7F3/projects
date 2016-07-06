#ifndef _system_status_
#define _system_status_

#include "setup.h"
#include "UTMdef.h"
// #include <QT\qdatastream.h>
#include <QtNetwork/QtNetwork>
#include <QtCore/QSignalMapper>

#include <stdio.h>
#include <iostream>
#include <sstream>
#include <string>
using namespace std;

#pragma pack(4)

// System Status
typedef struct {
	bool			running;
	sUTMCoord		utmCoord;
	float			altitude;
	float			heading;

	bool			NFOVAvailable;
	uint32_t		currentNFOVCommandID;

	uint32_t        	numValidROIS;
	uint32_t		roiId[MAX_NUM_ROIS];
	uint32_t		roiState[MAX_NUM_ROIS];
} sSystemStatus;
QDataStream& operator << (QDataStream& s, const sSystemStatus& x);
QDataStream& operator >> (QDataStream& s, sSystemStatus& x);
stringstream& operator << (stringstream& s, const sSystemStatus& x);
stringstream& operator >> (stringstream& s, sSystemStatus& x);

ostream& operator<< (ostream& s, const sSystemStatus& x);

#endif
