#ifndef _roi_track_update_
#define _roi_track_update_

#include "setup.h"
#include "ROITrackElement.h"
//#include <QT\qdatastream.h>
#include <QtNetwork/QtNetwork>
#include <QtCore/QSignalMapper>

#include <stdio.h>
#include <iostream>
#include <sstream>
#include <string>
using namespace std;

#pragma pack(4)

typedef struct {
	uint32_t		regionID;
	uint32_t		trackUpdatePriority;
	uint32_t		numTracks;
	uint32_t		numChips;
	sTrackElement	tracks[MAX_NUM_TRACKS];
} sROITrackUpdate;
QDataStream& operator << (QDataStream& s, const sROITrackUpdate& x);
QDataStream& operator >> (QDataStream& s, sROITrackUpdate& x);
stringstream& operator << (stringstream& s, const sROITrackUpdate& x);
stringstream& operator >> (stringstream& s, sROITrackUpdate& x);

ostream& operator<< (ostream& s, const sROITrackUpdate& x);

#endif
