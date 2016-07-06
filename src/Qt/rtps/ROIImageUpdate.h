#ifndef _roi_image_update_
#define _roi_image_update_

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

// ROI Image Update
typedef struct {
	uint32_t		regionID;
	uint32_t		imageUpdatePriority;
	sUTMCorners		utmCorners; 
	uint32_t		imageSize;
	stringstream		imageData;
} sROIImageUpdate;
QDataStream& operator << (QDataStream& s, const sROIImageUpdate& x);
QDataStream& operator >> (QDataStream& s, sROIImageUpdate& x);
stringstream& operator << (stringstream& s, const sROIImageUpdate& x);
stringstream& operator >> (stringstream& s, sROIImageUpdate& x);
ostream& operator<< (ostream& s, const sROIImageUpdate& x);



#endif
