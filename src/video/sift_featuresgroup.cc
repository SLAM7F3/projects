// ==========================================================================
// Sift_Featuresgroup class member function definitions
// ==========================================================================
// Last modified on 3/17/11; 3/18/11
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "video/photodbfuncs.h"
#include "video/sift_featuresgroup.h"

using std::cout;
using std::endl;
using std::ofstream;
using std::map;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void sift_featuresgroup::allocate_member_objects()
{
   photosgroup_siftfeatures_map_ptr=new PHOTOSGROUP_SIFTFEATURES_MAP;
}		       

void sift_featuresgroup::initialize_member_objects()
{
}

sift_featuresgroup::sift_featuresgroup(int ID) 
{
   allocate_member_objects();
   initialize_member_objects();
   this->ID=ID;
}

// Copy constructor:

sift_featuresgroup::sift_featuresgroup(const sift_featuresgroup& fg) 
{
   allocate_member_objects();
   initialize_member_objects();
   docopy(fg);
}

sift_featuresgroup::~sift_featuresgroup()
{
   delete photosgroup_siftfeatures_map_ptr;
}

// ---------------------------------------------------------------------
void sift_featuresgroup::docopy(const sift_featuresgroup& fg)
{
//   cout << "inside sift_featuresgroup::docopy()" << endl;
}

// Overload = operator:

sift_featuresgroup& sift_featuresgroup::operator= (const sift_featuresgroup& fg)
{
   if (this==&fg) return *this;
   docopy(fg);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const sift_featuresgroup& fg)
{
   outstream << "inside sift_featuresgroup::operator<<" << endl;
   outstream << endl;
   return outstream;
}


// =====================================================================
// 
// =====================================================================

void sift_featuresgroup::add_siftfeature(sift_feature* sift_feature_ptr)
{
//   cout << "inside sift_featuresgroup::add_siftfeature(), photo_ID = " 
//        << photo_ID  << endl;
   
   int photo_ID=sift_feature_ptr->get_photo_ID();
   PHOTOSGROUP_SIFTFEATURES_MAP::iterator iter=
      photosgroup_siftfeatures_map_ptr->find(photo_ID);

   PHOTO_SIFTFEATURES_MAP* photo_siftfeatures_map_ptr=NULL;
   if (iter==photosgroup_siftfeatures_map_ptr->end())
   {
      photo_siftfeatures_map_ptr=new PHOTO_SIFTFEATURES_MAP;
      (*photosgroup_siftfeatures_map_ptr)[photo_ID]=photo_siftfeatures_map_ptr;
   }
   else
   {
      photo_siftfeatures_map_ptr=iter->second;
   }

   int feature_ID=sift_feature_ptr->get_ID();
//   cout << "feature_ID = " << feature_ID << endl;
   (*photo_siftfeatures_map_ptr)[feature_ID]=sift_feature_ptr;
//   cout << "photo_siftfeatures_map_ptr->size() = "
//        << photo_siftfeatures_map_ptr->size() << endl;
}

// ---------------------------------------------------------------------
int sift_featuresgroup::get_n_sift_features(int photo_ID) const
{
//   cout << "inside get_n_sift_features(), photo_ID = " << photo_ID << endl;
   PHOTOSGROUP_SIFTFEATURES_MAP::const_iterator iter=
      photosgroup_siftfeatures_map_ptr->find(photo_ID);

   if (iter==photosgroup_siftfeatures_map_ptr->end())
   {
      return 0;
   }
   PHOTO_SIFTFEATURES_MAP* photo_siftfeatures_map_ptr=iter->second;
   return photo_siftfeatures_map_ptr->size();
}

// ---------------------------------------------------------------------
int sift_featuresgroup::count_all_sift_features() const
{
   int n_total_sift_features=0;

   for (PHOTOSGROUP_SIFTFEATURES_MAP::const_iterator iter=
           photosgroup_siftfeatures_map_ptr->begin();
        iter != photosgroup_siftfeatures_map_ptr->end(); iter++)
   {
      int photo_ID=iter->first;
      int curr_n_sift_features=get_n_sift_features(photo_ID);
      n_total_sift_features += curr_n_sift_features;
      cout << "photo_ID = " << photo_ID
           << " n_sift_features = " << curr_n_sift_features << endl;
   }
   return n_total_sift_features;
}

// ---------------------------------------------------------------------
// Member function write_SQL_insert_features_commands() generates
// SQL commands which populates columns in the sift_features table of
// the TOC database.

void sift_featuresgroup::write_SQL_insert_features_commands(
   int photo_ID,string SQL_filename) const
{
//   cout << "inside sift_featuresgroup::write_SQL_insert_features_commands()" << endl;

   PHOTOSGROUP_SIFTFEATURES_MAP::const_iterator iter=
      photosgroup_siftfeatures_map_ptr->find(photo_ID);

   if (iter==photosgroup_siftfeatures_map_ptr->end())
   {
      return;
   }
   PHOTO_SIFTFEATURES_MAP* photo_siftfeatures_map_ptr=iter->second;

   ofstream SQL_stream;
   filefunc::appendfile(SQL_filename,SQL_stream);

   for (PHOTO_SIFTFEATURES_MAP::const_iterator feature_iter=
           photo_siftfeatures_map_ptr->begin();
        feature_iter != photo_siftfeatures_map_ptr->end(); feature_iter++)
   {
      sift_feature* sift_feature_ptr=feature_iter->second;
      string SQL_command=photodbfunc::generate_insert_sift_feature_SQL_command(
         sift_feature_ptr);
//   cout << SQL_command << endl;
//   outputfunc::enter_continue_char();

      SQL_stream << SQL_command << endl;
   }

   filefunc::closefile(SQL_filename,SQL_stream);
}
