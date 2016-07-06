#ifndef _ROITrackElement_
#define _ROITrackElement_

#include "setup.h"
#include "UTMdef.h"
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
typedef struct {
	sUTMCoord       utmCoord; 
	uint32_t		trackID;
	uint32_t		chipSize;
	stringstream	chipData;
	sUTMCorners		utmCorners; 
} sTrackElement;
QDataStream& operator << (QDataStream& s, const sTrackElement& x);
QDataStream& operator >> (QDataStream& s, sTrackElement& x);
stringstream& operator << (stringstream& s, const sTrackElement& x);
stringstream& operator >> (stringstream& s, sTrackElement& x);
ostream& operator<< (ostream& s, const sTrackElement& x);

#endif
