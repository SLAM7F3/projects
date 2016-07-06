// ==========================================================================
// Postgis_Databases_Group class member function definitions
// ==========================================================================
// Last modified on 10/26/11; 5/4/12; 5/23/12
// ==========================================================================

#include "osg/osgGIS/postgis_databases_group.h"

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

void postgis_databases_group::allocate_member_objects()
{
}		       

void postgis_databases_group::initialize_member_objects()
{
   postgis_database_ptr=NULL;
}

postgis_databases_group::postgis_databases_group() :
   gis_databases_group()
{
   allocate_member_objects();
   initialize_member_objects();
}

postgis_databases_group::~postgis_databases_group()
{
   if (postgis_database_ptr != NULL) postgis_database_ptr->disconnect();
   delete postgis_database_ptr;
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

postgis_database* postgis_databases_group::get_postgis_database_ptr(int i)
{
   gis_database* gis_database_ptr=get_gis_database_ptr(i);
   return dynamic_cast<postgis_database*>(gis_database_ptr);
}

// --------------------------------------------------------------------------
// Member function get_gis_database_ptrs() fills and returns an
// STL vector with pointers to all gis databases within
// *databases_map_ptr.

vector<postgis_database*> postgis_databases_group::get_postgis_database_ptrs()
{
   vector<gis_database*> gis_database_ptrs=get_gis_database_ptrs();
   vector<postgis_database*> postgis_database_ptrs;
   for (unsigned int i=0; i<gis_database_ptrs.size(); i++)
   {
      postgis_database_ptrs.push_back(
         dynamic_cast<postgis_database*>(gis_database_ptrs[i]));
   }
   
   return postgis_database_ptrs;
}

// ==========================================================================
// GIS database access member functions
// ==========================================================================

// Member function generate_postgis_database_from_GISlayer_IDs()
// searches takes pass numbers within STL vector GISlayer_IDs.  It
// searches the pass labeled by GISlayer_IDs[input_GISlayer_index] for
// hostname, databasename and username information.  If found, this
// method dynamically instantiates postgis_database_ptr. It fills the
// database's gispoints_table with the pass' gispoints_tablename
// information.

postgis_database* 
postgis_databases_group::generate_postgis_database_from_GISlayer_IDs(
   PassesGroup& passes_group,const vector<int>& GISlayer_IDs,
   int input_GISlayer_index)
{
//   cout << "inside postgis_databases_group::generate_postgis_database_from_GISlayer_IDs() #1" << endl;
   
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
      postgis_database_ptr=new postgis_database(
         hostname,database_name,username);
      gis_database_ptr=dynamic_cast<gis_database*>(postgis_database_ptr);
      enable_Gis_database_access(gis_database_ptr);
   }
   
// UGLY HACK !!! As of 5/4/12, we believe that this (and the next)
// method needs a major overhaul.  As of 5/4/12, we assume that the
// zeroth postgis database specified within a --GIS_layer always
// corresponds to world_GIS.pkg.  Any other input postgis database
// (e.g. imagery_metadata.pkg) is assumed to NOT have
// GISlines_tablenames[] which needs to be read in.  So we hardwire the 
// requirement input_GISlayer_index==0 into the following conditional:

   if (GISlayer_IDs.size() > 0 && gis_database_ptr != NULL &&
   input_GISlayer_index==0)
   {
      vector<string> GISpoints_tablenames=
         passes_group.get_pass_ptr(GISlayer_IDs[0])->
         get_PassInfo_ptr()->get_gispoints_tablenames();
      for (unsigned int t=0; t<GISpoints_tablenames.size(); t++)
      {
         gis_database_ptr->pushback_GISpoint_tablename(
            GISpoints_tablenames[t]);
         cout << "t = " << t 
              << " tablename = " << GISpoints_tablenames[t] << endl;
      }

      vector<string> GISlines_tablenames=
         passes_group.get_pass_ptr(GISlayer_IDs[0])->
         get_PassInfo_ptr()->get_gislines_tablenames();
//      cout << "GISlines_tablenames = " << endl;
//      templatefunc::printVector(GISlines_tablenames);

      for (unsigned int t=0; t<GISlines_tablenames.size(); t++)
      {
         gis_database_ptr->pushback_GISlines_tablename(
            GISlines_tablenames[t]);
      }
      if (GISlines_tablenames.size() > 0)
      {
         gis_database_ptr->read_table(GISlines_tablenames[0]);
      }
   }

//   cout << "gis_database_ptr = " << gis_database_ptr << endl;
   
   return postgis_database_ptr;
}

// ---------------------------------------------------------------------
vector<postgis_database*>
postgis_databases_group::generate_postgis_databases_from_GISlayer_IDs(
   PassesGroup& passes_group,const vector<int>& GISlayer_IDs)
{
//   cout << "inside postgis_databases_group::generate_postgis_databases_from_GISlayer_IDs() #2" << endl;
   
   vector<postgis_database*> postgis_database_ptrs;
   for (unsigned int g=0; g<GISlayer_IDs.size(); g++)
   {
      string hostname="";
      string database_name="";
      string username="";

      hostname=passes_group.get_pass_ptr(GISlayer_IDs[g])->
         get_PassInfo_ptr()->get_PostGIS_hostname();
      database_name=passes_group.get_pass_ptr(GISlayer_IDs[g])->
         get_PassInfo_ptr()->get_PostGIS_database_name();
      username=passes_group.get_pass_ptr(GISlayer_IDs[g])->
         get_PassInfo_ptr()->get_PostGIS_username();

//   cout << "hostname = " << hostname << endl;
//   cout << "database_name = " << database_name << endl;
//   cout << "username = " << username << endl;

      if (hostname.size() > 0 && database_name.size() > 0 && username.size() 
      > 0)
      {
         postgis_database_ptr=new postgis_database(
            hostname,database_name,username);
         postgis_database_ptrs.push_back(postgis_database_ptr);
         gis_database_ptr=dynamic_cast<gis_database*>(postgis_database_ptr);
         enable_Gis_database_access(gis_database_ptr);
      }
   
      if (GISlayer_IDs.size() > 0 && gis_database_ptr != NULL)
      {
         vector<string> GISpoints_tablenames=
            passes_group.get_pass_ptr(GISlayer_IDs[GISlayer_IDs[0]])->
            get_PassInfo_ptr()->get_gispoints_tablenames();
         for (unsigned int t=0; t<GISpoints_tablenames.size(); t++)
         {
            gis_database_ptr->pushback_GISpoint_tablename(
               GISpoints_tablenames[t]);
            cout << "t = " << t 
                 << " tablename = " << GISpoints_tablenames[t] << endl;
         }

         vector<string> GISlines_tablenames=
            passes_group.get_pass_ptr(GISlayer_IDs[GISlayer_IDs[0]])->
            get_PassInfo_ptr()->get_gislines_tablenames();
         cout << "GISlines_tablenames = " << endl;
         templatefunc::printVector(GISlines_tablenames);

         for (unsigned int t=0; t<GISlines_tablenames.size(); t++)
         {
            gis_database_ptr->pushback_GISlines_tablename(
               GISlines_tablenames[t]);
         }
         if (GISlines_tablenames.size() > 0)
         {
            gis_database_ptr->read_table(GISlines_tablenames[GISlayer_IDs[0]]);
         }
      }

   } // loop over index g labeling postgis databases

   return postgis_database_ptrs;
}

// --------------------------------------------------------------------------
postgis_database* postgis_databases_group::initialize_database_and_table(
   string DatabaseName,string TableName)
{
   return dynamic_cast<postgis_database*>(
      gis_databases_group::initialize_database_and_table(
         DatabaseName,TableName));
}

