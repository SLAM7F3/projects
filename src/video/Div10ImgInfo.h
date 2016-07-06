
#include "video/pa_struct.h"

// Group 104 IRIG structs

typedef union tTimeHiBCD
{
	unsigned long	ul;
	struct {
		char	byMinOne:4;
		char	byMinTen:4;
		char	byHrOne:4;
		char	byHrTen:4;
		char	byDayOne:4;
		char	byDayTen:4;
		char	byDayHun:4;
		char	byYrOne:4;
	};
}* tTimeHiBCDP;

typedef union tTimeLoBCD
{
	unsigned long	ul;
	struct {
		char	byUsecOne:4;
		char	byUsecTen:4;
		char	byUsecHun:4;
		char	byMsecOne:4;
		char	byMsecTen:4;
		char	byMsecHun:4;
		char	bySecOne:4;
		char	bySecTen:4;
	};
}* tTimeLoBCDP;


typedef union tTimeFullBCD
{
	//
	// Data members...
	//
	long double	ull;	// All in one hi & lo
	struct {
		union tTimeLoBCD	lo;
		union tTimeHiBCD	hi;
	};
}* tTimeFullBCDP;

//
// Per-Image trailer info structure for Div10 save-to-disk.
//

typedef struct tDiv10ImgInfo 
{
      unsigned long dwFrameSeqNum;		
      // Sequence Number of this frame since the grab was begun
      double dblUsecArrivalTime; 
      // System time that frame arrived in microseconds
      tTimeFullBCD tfbcd; // IRIG card BCD time stamp
      unsigned int uUsecFramePeriod;	
      // Frame period for which integration time is a component
      unsigned int uUsecIntegTime;		
      // Basler A102K integration time in usec
      unsigned int uIris; // V2LC Lens controller aperture setting
      //
      // Variable GPS/IMU info array...
      //
      unsigned int uCntPAS;  // Count of valid "pa_struct" which follow
      pa_struct	asPA[5];     // Array of "pa_struct"s
};
