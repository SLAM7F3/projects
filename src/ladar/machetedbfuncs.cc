// ==========================================================================
// Machetedbfuncs namespace method definitions
// ==========================================================================
// Last modified on 4/4/12; 4/5/12
// ==========================================================================

#include <iostream>
#include <vector>
#include "postgres/gis_database.h"
#include "ladar/machetedbfuncs.h"
#include "general/stringfuncs.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;

namespace machetedbfunc
{

// ==========================================================================
// Database metadata insertion methods
// ==========================================================================

// Method insert_file_metadata()

   bool insert_file_metadata(
      gis_database* gis_database_ptr,
      int campaign_ID,int sortie_ID,int pass_ID,string file_type,string URL,
      double insertion_epoch,string insertion_UTC)
   {
//      cout << "inside machetedbfunc::insert_file_metadata()" << endl;


      string SQL_command="insert into data_files ";
      SQL_command += "(campaign_ID,sortie_ID,pass_ID,type,URL,import_utc,import_epoch) ";
      SQL_command += "values( ";
      SQL_command += stringfunc::number_to_string(campaign_ID)+",";
      SQL_command += stringfunc::number_to_string(sortie_ID)+",";
      SQL_command += stringfunc::number_to_string(pass_ID)+",";
      SQL_command += "'"+file_type+"',";
      SQL_command += "'"+URL+"',";
      SQL_command += "'"+insertion_UTC+"',";
      SQL_command += stringfunc::number_to_string(insertion_epoch);
      SQL_command += ");";
      cout << SQL_command << endl;

      vector<string> insert_commands;
      insert_commands.push_back(SQL_command);
      gis_database_ptr->set_SQL_commands(insert_commands);
      return gis_database_ptr->execute_SQL_commands();
   }


} // machetedbfunc namespace







