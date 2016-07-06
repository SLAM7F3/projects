// ==========================================================================
// Header file for FAKE_IFOV structure
// ==========================================================================
// Last modified on 3/17/04
// ==========================================================================

#ifndef FAKEIFOV_H
#define FAKEIFOV_H

struct fakeIFOV
{
//      unsigned short data[7];        // 6 bytes
//      unsigned short data[100];        // 2400 bytes
      unsigned short data[1200];        // 2400 bytes
//      int deltaTime;			// 4 bytes
//      double applanixTime;		// 8 bytes
};

#endif // fakeIFOV.h

