// ==========================================================================
// Header file for machetedbfunc namespace
// ==========================================================================
// Last modified on 4/4/12; 4/5/12
// ==========================================================================

#ifndef MACHETEDBFUNCS_H
#define MACHETEDBFUNCS_H

class gis_database;

namespace machetedbfunc
{

// Data file metadata insertion methods

   bool insert_file_metadata(
      gis_database* gis_database_ptr,
      int campaign_ID,int sortie_ID,int pass_ID,
      std::string file_type,std::string URL,
      double insertion_epoch,std::string insertion_UTC);

// ==========================================================================
// Inlined methods:
// ==========================================================================

} // machetedbfunc namespace

#endif // machetedbfuncs.h

