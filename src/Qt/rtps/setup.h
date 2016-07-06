#ifndef SETUP_H
#define SETUP_H

#ifdef _WIN32
typedef unsigned char uint8_t;
typedef char int8_t;
typedef unsigned short int uint16_t;
typedef short int int16_t;
typedef unsigned int uint32_t;
typedef int int32_t;
typedef unsigned long long uint64_t;
typedef long long int64_t;
#else
#include <stdint.h>
#endif

#define DEFAULT_MQ_SERVER_NAME "127.0.0.1"
#define DEFAULT_MQ_SERVER_PORT "61616"

#define DEFAULT_GROUND_STATION_NAME "127.0.0.1"
#define DEFAULT_GROUND_STATION_PORT_RCV "60001"
#define DEFAULT_GROUND_STATION_PORT_SND "60002"

#define MAX_NUM_TRACKS 500
#define MAX_NUM_ROIS 8
#define MAX_NUM_CONSOLE_MESSAGES 100
#define MIN_ALLOCATION_TIME_NFOV_IN_uS 15000000
static char _UTMZone[4] = "14S";
static int _FORCEDZONENUMBER = 14;

#endif
