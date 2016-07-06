// ==========================================================================
// Program MAPTEST
// ==========================================================================
// Last updated on 1/5/07
// ==========================================================================

#include <iostream>
#include <iterator>
#include <ext/hash_map>
#include <map>
#include <math.h>
#include <string>
#include <vector>
#include <osg/Quat>
#include <osg/Vec3>
#include "math/constants.h"
#include "general/filefuncs.h"
#include "kdtree/kdtreefuncs.h"
#include "math/mathfuncs.h"
#include "general/outputfuncs.h"
#include "osg/osgfuncs.h"
#include "datastructures/Quadruple.h"
#include "osg/osgGeometry/PolyLine.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "math/threevector.h"

#include "math/ltthreevector.h"

using std::cin;
using std::cout;
using std::endl;
using std::map;
using std::ofstream;
using std::string;
using std::vector;
      
bool equal_to(const threevector& V1,const threevector& V2) 
{
   return (nearly_equal(V1.get(0),V2.get(0)) &&
           nearly_equal(V1.get(1),V2.get(1)) &&
           nearly_equal(V1.get(2),V2.get(2)));
}

bool less_than(const threevector& V1,const threevector& V2)
{

   if (V1.get(0) < V2.get(0))
   {
      return true;
   }
   else if (V1.get(0) > V2.get(0))
   {
      return false;
   }
   else
   {
      if (V1.get(1) < V2.get(1))
      {
         return true;
      }
      else if (V1.get(1) > V2.get(1))
      {
         return false;
      }
      else
      {
         if (V1.get(2) < V2.get(2))
         {
            return true;
         }
         else
         {
            return false;
         }
      }
   }
}
typedef std::map<threevector,int,ltthreevector > LINES_MAP;

LINES_MAP longitude_lines_map;

/*
LINES_MAP::iterator
long_lat_lat_to_map_iterator(
   double longitude,double lat_start,double lat_stop)
{
   for (LINES_MAP::iterator map_iter=
           longitude_lines_map.begin(); map_iter !=
           longitude_lines_map.end(); ++map_iter)
   {
      threevector curr_lll(map_iter->first);
      if (nearly_equal(longitude,curr_lll.get(0)) &&
          nearly_equal(lat_start,curr_lll.get(1)) &&
          nearly_equal(lat_stop,curr_lll.get(2)))
      {
         return map_iter;
      }
   } // loop over nodes in *longitude_lines_list_ptr
   return NULL;
}
*/

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);
   cout.precision(10);

   longitude_lines_map[threevector(0,0,0)]=1;
   longitude_lines_map[threevector(0,0,1)]=2;
   longitude_lines_map[threevector(0,1,0)]=3;
   longitude_lines_map[threevector(1,0,0)]=4;
   longitude_lines_map[threevector(0,1,2)]=5;
   longitude_lines_map[threevector(4,3,0)]=6;
   longitude_lines_map[threevector(0,0,1)]=7;

   for (LINES_MAP::iterator map_iter=
           longitude_lines_map.begin(); map_iter != longitude_lines_map.end();
        ++map_iter)
   {
      cout << "key = " << map_iter->first
           << " value = " << map_iter->second << endl;
   }
   
   LINES_MAP::iterator map_iter=
      longitude_lines_map.find(threevector(0,1,2));
   if (map_iter==NULL)
   {
      cout << "No entry found within map" << endl;
   }
   else
   {
      cout << "map_iter->first = " << map_iter->first
           << " map_iter->Second = " << map_iter->second << endl;
      map_iter->second=17;
   }

   exit(-1);
 
   cout << "====================================================" << endl;

   for (LINES_MAP::iterator map_iter=
           longitude_lines_map.begin(); map_iter != longitude_lines_map.end();
        ++map_iter)
   {
      cout << "key = " << map_iter->first
           << " value = " << map_iter->second << endl;

      LINES_MAP::iterator found_iter=
         longitude_lines_map.find(threevector(0,1,0));

      if (found_iter==longitude_lines_map.end())
      {
         cout << "found_iter=NULL" << endl;
      }
      else
      {
         cout << "found key = " << found_iter->first
              << " found value = " << found_iter->second << endl;
         longitude_lines_map.erase(found_iter);
      }
   }
   

   cout << "After erasing found element" << endl;
   
   for (LINES_MAP::iterator map_iter=
           longitude_lines_map.begin(); map_iter != 
           longitude_lines_map.end(); ++map_iter)
   {
      cout << "key = " << map_iter->first
           << " value = " << map_iter->second << endl;
   }

/*
   cout << "(1,1,1) equalto (1,1,1) = "
        << equal_to(threevector(1,1,1),threevector(1,1,1)) << endl;
   cout << "(1,1,1) equalto (1,1,2) = "
        << equal_to(threevector(1,1,1),threevector(1,1,2)) << endl;
*/ 

}

   
