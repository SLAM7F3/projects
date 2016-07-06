// ==========================================================================
// Header file for postgis_databases_group class
// ==========================================================================
// Last modified on 5/16/10; 10/26/11; 5/4/12
// ==========================================================================

#ifndef POSTGIS_DATABASES_GROUP_H
#define POSTGIS_DATABASES_GROUP_H

#include <map>
#include <string>
#include <vector>

class PassesGroup;

#include "postgres/gis_databases_group.h"
#include "general/ltstring.h"
#include "osg/osgGIS/postgis_database.h"

class postgis_databases_group : public gis_databases_group
{

  public:

// Initialization, constructor and destructor functions:

   postgis_databases_group();

// On 11/13/01, we learned from Tara Dennis that base class
// destructors ought to always be declared as virtual so that they
// will automatically be called by inherited class destructors:

   virtual ~postgis_databases_group();

// Set & get member functions:

   postgis_database* get_postgis_database_ptr(int i);
   std::vector<postgis_database*> get_postgis_database_ptrs();

// PostGIS database access member functions:

   postgis_database* generate_postgis_database_from_GISlayer_IDs(
      PassesGroup& passes_group,const std::vector<int>& GISlayer_IDs,
      int input_GISlayer_index=0);
   std::vector<postgis_database*> generate_postgis_databases_from_GISlayer_IDs(
      PassesGroup& passes_group,const std::vector<int>& GISlayer_IDs);
   postgis_database* initialize_database_and_table(
      std::string DatabaseName,std::string TableName);

  private:

   postgis_database* postgis_database_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const postgis_databases_group& m);

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

/*
inline void postgis_databases_group::set_base_URL(std::string URL)
{
   base_URL=URL;
}
*/

#endif  // postgis_databases_group.h



