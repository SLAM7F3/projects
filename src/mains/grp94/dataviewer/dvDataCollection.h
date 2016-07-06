// ==========================================================================
// Header file for dvDataCollection class
// ==========================================================================
// Last updated on 6/6/04
// ==========================================================================

/********************************************************************
 *
 * Name: dvDataCollection.h
 * Author: Luke Skelly
 *
 * Description:
 * a container class for dvData
 **********************************************************************/

#ifndef DVDATACOLLECTION_H
#define DVDATACOLLECTION_H

#include <iostream>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "dvData.h"

using namespace std;
using namespace boost;

class dvDataCollection:public dvData
{

  public:

   dvDataCollection();
   virtual int glDraw(vector<float> & camera_position,
                      bool force_redraw=true,clock_t deadline=0,
                      CFrustum * f=NULL);
   virtual int glDraw(bool force_redraw=true,clock_t deadline=0,
                      CFrustum * f=NULL);
   void add(const shared_ptr<dvData> & data);
   long size() const;
   void clear();
   int getNumberOfElements() const;
   virtual void setColormap(dvColormap* c);
   virtual void setConstraint(dvDataConstraint* d);
   virtual void modColorCode(int mod);
   virtual void setColorCode(int c);
   virtual void data_dump(FILE * file);
   virtual bool dvDataCollection::isChanged();
   virtual void resetConstraint();

  protected:

   void force();
   shared_ptr<dvData> & getData(long index);
   vector< shared_ptr<dvData> > collection;
   vector< pair<float, long> > sorter;
   vector<bool> finished_force;
   long finished_force_pos;

//    int modulo(int i,int n);
};

/*
// ---------------------------------------------------------------------
// Method modulo takes returns i mod n provided n > 0.  Unlike
// C++'s built in remainder function, integer i can assume any
// positive, negative or zero value...

inline int dvDataCollection::modulo(int i,int n)
{
   int j;
   
   if (n < 0)
   {
      std::cout << "Error inside modulo method in dVDataCollection !" 
                << std::endl;
      std::cout << "i = " << i << " n = " << n << std::endl;
      return 0;
   }
   else
   {
      if (i >= 0)
      {
         return i%n;
      }
      else
      {
         j=i+n*(int(abs(i)/n)+1);
         return j%n;
      }
   }
}
*/

#endif // DVDATACOLLECTION_H


