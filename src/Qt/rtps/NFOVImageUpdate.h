#ifndef _nfov_image_update_
#define _nfov_image_update_

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

//NFOV Image Update
typedef struct {
	sUTMCorners		utmCorners; 
	uint32_t		imageSize;
	stringstream	imageData;
} sNFOVImageUpdate;
QDataStream& operator << (QDataStream& s, const sNFOVImageUpdate& x);
QDataStream& operator >> (QDataStream& s, sNFOVImageUpdate& x);
stringstream& operator << (stringstream& s, const sNFOVImageUpdate& x);
stringstream& operator >> (stringstream& s, sNFOVImageUpdate& x);
ostream& operator<< (ostream& s, const sNFOVImageUpdate& x);

#endif
