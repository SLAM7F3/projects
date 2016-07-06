// -------------------------------------------------------------------
// Last updated on 10/16/06; 10/19/06; 10/27/06
// -------------------------------------------------------------------

#ifndef __SKS_ANNOTATE_H__
#define __SKS_ANNOTATE_H__

#include <string>
#include "math/genmatrix.h"
#include "math/twovector.h"

class database;

namespace sksfunc
{
   std::string AddSksAnnotation (
      database* old_isds_database_ptr,
      double lat, double lon,std::string labelName);
   void DeleteSksAnnotation(database* old_isds_database_ptr,
                            std::string SKS_label_ID);
   void UpdateSksAnnotationName(database* old_isds_database_ptr,
                                std::string SKS_label_ID,
                                std::string newName);
} // sksfunc namespace


#endif
