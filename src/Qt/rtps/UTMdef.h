#ifndef _UTMdef_
#define _UTMdef_

#include "setup.h"
// #include <QT\qdatastream.h>
#include <QtNetwork/QtNetwork>
#include <QtCore/QSignalMapper>

#include <stdio.h>
#include <iostream>
#include <sstream>
#include <string>
using namespace std;

#pragma pack(4)

typedef struct  {
	double			Northing;
	double			Easting;
	char			Zone[10];
} sUTMCoord;
QDataStream& operator << (QDataStream& s, const sUTMCoord& x);
QDataStream& operator >> (QDataStream& s, sUTMCoord& x);
stringstream& operator << (stringstream& s, const sUTMCoord& x);
stringstream& operator >> (stringstream& s, sUTMCoord& x);
ostream& operator<< (ostream& s, const sUTMCoord& x);

typedef struct  {
	double			Northing[4];
	double			Easting[4];
	char			Zone[10];
} sUTMCorners;
QDataStream& operator << (QDataStream& s, const sUTMCorners& x);
QDataStream& operator >> (QDataStream& s, sUTMCorners& x);
stringstream& operator << (stringstream& s, const sUTMCorners& x);
stringstream& operator >> (stringstream& s, sUTMCorners& x);
ostream& operator<< (ostream& s, const sUTMCorners& x);

#endif
