#ifndef _wfov_image_update_
#define _wfov_image_update_

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

// WFOV Image Update
typedef struct {
	sUTMCorners		utmCorners;
	uint32_t		imageSize;
	stringstream	imageData;
} sWFOVImageUpdate;
QDataStream& operator << (QDataStream& s, const sWFOVImageUpdate& x);
QDataStream& operator >> (QDataStream& s, sWFOVImageUpdate& x);
stringstream& operator << (stringstream& s, const sWFOVImageUpdate& x);
stringstream& operator >> (stringstream& s, sWFOVImageUpdate& x);
ostream& operator<< (ostream& s, const sWFOVImageUpdate& x);

#endif
