// ==========================================================================
// Gis_Databases_Group class member function definitions
// ==========================================================================
// Last modified on 5/16/10; 10/26/11
// ==========================================================================

#include "postgres/gis_databases_group.h"
#include "passes/PassesGroup.h"

using std::cout;
using std::endl;
using std::map;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void gis_databases_group::allocate_member_objects()
{
   databases_map_ptr=new DATABASES_MAP;
   strings_map_ptr=new STRINGS_MAP;
}		       

void gis_databases_group::initialize_member_objects()
{
   gis_database_ptr=NULL;
}

gis_databases_group::gis_databases_group() 
{
   allocate_member_objects();
   initialize_member_objects();
}

gis_databases_group::~gis_databases_group()
{
   delete databases_map_ptr;
   delete strings_map_ptr;

   if (gis_database_ptr != NULL) gis_database_ptr->disconnect();
   delete gis_database_ptr;
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

gis_database* gis_databases_group::get_gis_database_ptr(int i)
{
   vector<gis_database*> gis_databases_group=get_gis_database_ptrs();
   return gis_databases_group[i];
}

// ==========================================================================
// Gis database access member functions
// ==========================================================================

// Member function generate_gis_database_from_GISlayer_IDs()
// takes pass numbers within STL vector GISlayer_IDs.  It
// searches the pass labeled by GISlayer_IDs[input_GISlayer_index] for
// hostname, databasename and username information.  If found, this
// method dynamically instantiates gis_database_ptr. It fills the
// database's gispoints_table with the pass' gispoints_tablename
// information.

gis_database* 
gis_databases_group::generate_gis_database_from_GISlayer_IDs(
   PassesGroup& passes_group,const vector<int>& GISlayer_IDs,
   int input_GISlayer_index)
{
//   cout << "inside gis_databases_group::generate_gis_database_from_GISlayer_IDs()" << endl;
//   cout << "input_GISlayer_index = " << input_GISlayer_index << endl;
//   cout << "GISlayer_IDs.size() = " << GISlayer_IDs.size() << endl;

   string hostname="";
   string database_name="";
   string username="";
   if (GISlayer_IDs.size() > 0)
   {
      hostname=passes_group.get_pass_ptr(GISlayer_IDs[input_GISlayer_index])->
         get_PassInfo_ptr()->get_PostGIS_hostname();
      database_name=passes_group.get_pass_ptr(
         GISlayer_IDs[input_GISlayer_index])->
         get_PassInfo_ptr()->get_PostGIS_database_name();
      username=passes_group.get_pass_ptr(GISlayer_IDs[input_GISlayer_index])->
         get_PassInfo_ptr()->get_PostGIS_username();
   }
//   cout << "hostname = " << hostname << endl;
//   cout << "database_name = " << database_name << endl;
//   cout << "username = " << username << endl;

   if (hostname.size() > 0 && database_name.size() > 0 && username.size() > 0)
   {
      gis_database_ptr=new gis_database(hostname,database_name,username);
      enable_Gis_database_access(gis_database_ptr);
   }
   
   if (GISlayer_IDs.size() > 0 && gis_database_ptr != NULL)
   {
//      cout << "n_passes = " << passes_group.get_n_passes() << endl;
      
//      Pass* pass_ptr=
//         passes_group.get_pass_ptr(GISlayer_IDs[input_GISlayer_index]);
//      PassInfo* PassInfo_ptr=pass_ptr->get_PassInfo_ptr();
      
/*
      vector<string> GISpoints_tablenames=
         PassInfo_ptr->get_gispoints_tablenames();
      for (int t=0; t<GISpoints_tablenames.size(); t++)
      {
         gis_database_ptr->pushback_GISpoint_tablename(
            GISpoints_tablenames[t]);
         cout << "t = " << t 
              << " tablename = " << GISpoints_tablenames[t] << endl;
      }

      vector<string> GISlines_tablenames=
         PassInfo_ptr->get_gislines_tablenames();
      cout << "GISlines_tablenames = " << endl;
      templatefunc::printVector(GISlines_tablenames);

      for (int t=0; t<GISlines_tablenames.size(); t++)
      {
         gis_database_ptr->pushback_GISlines_tablename(
            GISlines_tablenames[t]);
      }
      if (GISlines_tablenames.size() > 0)
      {
         gis_database_ptr->read_table(GISlines_tablenames[0]);
      }
*/

   }
   return gis_database_ptr;
}

// ---------------------------------------------------------------------
// Member function enable_Gis_database_access stores the input
// gis database pointer within member STL map *databases_map_ptr
// as a dependent variable with the database's name string as its
// independent accessor variable.

void gis_databases_group::enable_Gis_database_access(
   gis_database* gis_db_ptr)
{   
//   cout << "inside gis_databases_group::enable_Gis_database_access()"
//        << endl;
   
   if (gis_db_ptr != NULL)
   {
      (*databases_map_ptr)[gis_db_ptr->get_databasename()]=gis_db_ptr;
   }
}

// ---------------------------------------------------------------------
// Member function initialize_database_and_table()

gis_database* gis_databases_group::initialize_database_and_table(
   string DatabaseName,string TableName)
{
//   cout << "inside gis_databases_group::initialize_database_and_table()"
//        << endl;
   
   DATABASES_MAP::iterator database_iter=databases_map_ptr->
      find(DatabaseName);
   if (database_iter==databases_map_ptr->end())
   {
      cout << "Could not find database corresponding to DatabaseName = "
           << DatabaseName << endl;
      return NULL;
   }
   gis_database* gis_database_ptr=database_iter->second;
   
   if (gis_database_ptr == NULL || 
       !(gis_database_ptr->get_connection_status_flag())) return NULL;

   STRINGS_MAP::iterator string_iter=strings_map_ptr->find(TableName);
   if (string_iter != strings_map_ptr->end())
   {
      cout << "Already retrieved data from table = " << TableName << endl;
      return NULL;
   } 

   if (TableName.size() > 0)
   {
      gis_database_ptr->set_TableName(TableName);
   }

   return gis_database_ptr;
}

// --------------------------------------------------------------------------
// Member function get_gis_database_ptrs() fills and returns an
// STL vector with pointers to all gis databases within
// *databases_map_ptr.

vector<gis_database*> gis_databases_group::get_gis_database_ptrs()
{
   vector<gis_database*> gis_database_ptrs;
   
   for (DATABASES_MAP::iterator database_iter=databases_map_ptr->begin();
        database_iter != databases_map_ptr->end(); database_iter++)
   {
      gis_database* gis_database_ptr=database_iter->second;
      gis_database_ptrs.push_back(gis_database_ptr);
   }

   return gis_database_ptrs;
}

// --------------------------------------------------------------------------
// Member function record_tablename_as_read() saves the fact that data
// to the input TableName have been read in so that they never have to
// be read in again,

void gis_databases_group::record_tablename_as_read(string TableName)
{
   (*strings_map_ptr)[TableName]=true;
}

