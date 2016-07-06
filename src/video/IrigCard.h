// Last updatedon 8/16/05

// IrigCard.h: interface for the CIrigCard class.

typedef union tTimeHiBCD
{
      unsigned long ul;
      struct {
            unsigned char byMinOne:4;
            unsigned char byMinTen:4;
            unsigned char byHrOne:4;
            unsigned char byHrTen:4;
            unsigned char byDayOne:4;
            unsigned char byDayTen:4;
            unsigned char byDayHun:4;
            unsigned char byYrOne:4;
      };
};

typedef union tTimeLoBCD
{
      unsigned long ul;
      struct {
            unsigned char byUsecOne:4;
            unsigned char byUsecTen:4;
            unsigned char byUsecHun:4;
            unsigned char byMsecOne:4;
            unsigned char byMsecTen:4;
            unsigned char byMsecHun:4;
            unsigned char bySecOne:4;
            unsigned char bySecTen:4;
      };
};

typedef union tTimeFullBCD
{
      //
      // Data members...
      //
      unsigned long long ull;	// All in one hi & lo
      struct 
      {
            tTimeLoBCD	lo;
            tTimeHiBCD	hi;
      };
      //
      // Methods...
      //
      void CalcTimeInUsec( unsigned long long &ullUsec ) const
      {
         const unsigned long long million=1000000;
         unsigned int ii, mark, last, mult;
         //
         // Compute:  usec <==> sec
         //
         mult = 10;
         ullUsec = lo.ul & 0xF;	// Start with one's usec digit

         for (ii = 1; ii < 8; ii++) 
         {
            ullUsec += ((lo.ul >> (ii*4)) & 0xF) * mult;
            mult *= 10;
         }
         //
         // Minutes:  1 min == 60 sec * 1000000 usec/sec
         //
         mult = 1;
         last = 0;
         mark = 2;	// 1s & 10s digits

         for (ii = 0; ii < mark; ii++) 
         {
            ullUsec +=  ((hi.ul >> (ii*4)) & 0xF) * mult * 60 * million;
            mult *= 10;
         }
         //
         // Hours:  1 hr == 60 min * 60 sec/min * 1000000 usec/sec
         //
         mult = 1;
         last = mark;
         mark += 2;	// 1s & 10s digits

         for ( ; ii < mark; ii++) {
            ullUsec += ((hi.ul >> (ii*4)) & 0xF) * mult * 60 * 60 * 
               million;
            mult *= 10;
         }
         //
         // Day of year:  1 day == 24 hr * 60 min/hr * 60 sec/min * 
	 //		1000000 usec/sec
         //
         // NOTE:  3 day digits
         //
         mult = 1;
         last = mark;
         mark += 3;	// 1s, 10s & 100s digits

         for ( ; ii < mark; ii++) {
            ullUsec += ((hi.ul >> (ii*4)) & 0xF) * mult * 24 * 60 * 60 * 
               million;
            mult *= 10;
         }
         //
         // Year of century:  1 year == 365/6 days/year * 24 hr * 
	 // 60 min/hr * 60 sec/min * 1000000 usec/sec
         //
         // NOTE:  Year one's digit
         //
         mult = 1;
         last = mark;
         mark += 1;	// 1s digit only

         for ( ; ii < mark; ii++) {
            ullUsec += ((hi.ul >> (ii*4)) & 0xF) * mult * 365 * 24 * 60 
               * 60 * million;
            mult *= 10;
         }
      }

      void CalcUsec( unsigned long &ulUsec )  // since start of second
      {
         //
         // Compute:  usec <==> sec
         //
         unsigned int mult = 10;
         ulUsec = lo.ul & 0xF;	// Start with one's usec digit

         for (unsigned int ii = 1; ii < 6; ii++) {
            ulUsec += ((lo.ul >> (ii*4)) & 0xF) * mult;
            mult *= 10;
         }
      }

      void CalcSec( unsigned long &ulSec )  // seconds since midnight
      {
         //
         // Compute:  usec <==> sec
         //
         ulSec = (lo.ul >> 24) & 0xF;	// Start with one's sec digit
         ulSec += ((lo.ul >> 28) & 0xF) * 10; // 10's digit
         ulSec += (hi.ul & 0xF) * 60; // minutes digit
         ulSec += ((hi.ul >> 4) & 0xF) * 600; // 10 minutes digit
         ulSec += ((hi.ul >> 8) & 0xF) * 3600; // hours digit
         ulSec += ((hi.ul >> 12) & 0xF) * 36000; // 10 hours digit
      }
      
}; // tTimeFullBCD typedef 

