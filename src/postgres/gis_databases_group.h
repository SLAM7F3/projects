// ==========================================================================
// Header file for gis_databases_group class
// ==========================================================================
// Last modified on 5/16/10; 10/26/11
// ==========================================================================

#ifndef GIS_DATABASES_GROUP_H
#define GIS_DATABASES_GROUP_H

#include <map>
#include <string>
#include <vector>

#include "general/ltstring.h"
#include "postgres/gis_database.h"

class PassesGroup;

class gis_databases_group 
{

  public:

   typedef std::map<std::string,gis_database*,ltstring > DATABASES_MAP;
   typedef std::map<std::string,bool,ltstring > STRINGS_MAP;

// Initialization, constructor and destructor functions:

   gis_databases_group();

// On 11/13/01, we learned from Tara Dennis that base class
// destructors ought to always be declared as virtual so that they
// will automatically be called by inherited class destructors:

   virtual ~gis_databases_group();

// Set & get member functions:

   gis_database* get_gis_database_ptr(int i);

// Gis database access member functions:

   gis_database* generate_gis_database_from_GISlayer_IDs(
      PassesGroup& passes_group,const std::vector<int>& GISlayer_IDs,
      int input_GISlayer_index=0);
   void enable_Gis_database_access(gis_database* gis_database_ptr);
   gis_database* initialize_database_and_table(
      std::string DatabaseName,std::string TableName);
   std::vector<gis_database*> get_gis_database_ptrs();
   void record_tablename_as_read(std::string TableName);

  protected:

   gis_database* gis_database_ptr;

  private:

   DATABASES_MAP* databases_map_ptr;
   STRINGS_MAP* strings_map_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const gis_databases_group& m);

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

/*
inline void gis_databases_group::set_base_URL(std::string URL)
{
   base_URL=URL;
}
*/

#endif  // gis_databases_group.h



