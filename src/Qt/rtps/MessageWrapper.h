#ifndef _message_wrapper_
#define _message_wrapper_

#include "setup.h"
// #include <QT\qdatastream.h>
#include <QtNetwork/QtNetwork>
#include <QtCore/QSignalMapper>
#include "NFOVCommand.h"
#include "NFOVImageUpdate.h"
#include "ROICommand.h"
#include "ROIImageUpdate.h"
#include "ROITrackUpdate.h"
#include "SystemStatus.h"
#include "Console.h"
#include "WFOVImageUpdate.h"

#include <stdlib.h>
#include <iostream>
#include <set>
#include <string>
#include <vector>
#include <sstream>
using namespace std;

#pragma pack(4)

// Link Message Wrapper
typedef struct  {
	uint32_t		messageID;
	uint32_t		length;
	stringstream	body;
} sMessageWrapper;
QDataStream& operator << (QDataStream& s, const sMessageWrapper& x);
QDataStream& operator >> (QDataStream& s, sMessageWrapper& x);
stringstream& operator << (stringstream& s, const sMessageWrapper& x);
stringstream& operator >> (stringstream& s, sMessageWrapper& x);
ostream& operator<< (ostream& s, const sMessageWrapper& x);

sMessageWrapper& operator << (sMessageWrapper& s, const sROICommand& x);
sMessageWrapper& operator >> (sMessageWrapper& s, sROICommand& x);

sMessageWrapper& operator << (sMessageWrapper& s, const sROITrackUpdate& x);
sMessageWrapper& operator >> (sMessageWrapper& s, sROITrackUpdate& x);

sMessageWrapper& operator << (sMessageWrapper& s, const sROIImageUpdate& x);
sMessageWrapper& operator >> (sMessageWrapper& s, sROIImageUpdate& x);

sMessageWrapper& operator << (sMessageWrapper& s, const sNFOVCommand& x);
sMessageWrapper& operator >> (sMessageWrapper& s, sNFOVCommand& x);

sMessageWrapper& operator << (sMessageWrapper& s, const sNFOVImageUpdate& x);
sMessageWrapper& operator >> (sMessageWrapper& s, sNFOVImageUpdate& x);

sMessageWrapper& operator << (sMessageWrapper& s, const sWFOVImageUpdate& x);
sMessageWrapper& operator >> (sMessageWrapper& s, sWFOVImageUpdate& x);

sMessageWrapper& operator << (sMessageWrapper& s, const sSystemStatus& x);
sMessageWrapper& operator >> (sMessageWrapper& s, sSystemStatus& x);

sMessageWrapper& operator << (sMessageWrapper& s, const sConsole& x);
sMessageWrapper& operator >> (sMessageWrapper& s, sConsole& x);

#endif
