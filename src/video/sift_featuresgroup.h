// ==========================================================================
// Header file for sift_featuresgroup class
// ==========================================================================
// Last modified on 3/17/11
// ==========================================================================

#ifndef SIFT_FEATURESGROUP_H
#define SIFT_FEATURESGROUP_H

#include <iostream>
#include <map>
#include "video/sift_feature.h"

class sift_featuresgroup 
{

  public:

   typedef std::map<int,sift_feature*> PHOTO_SIFTFEATURES_MAP;
//	Integer index corresponds to SIFT feature ID

   typedef std::map<int,PHOTO_SIFTFEATURES_MAP*> PHOTOSGROUP_SIFTFEATURES_MAP;
//	Integer index corresponds to PHOTO ID

// Initialization, constructor and destructor functions:

   sift_featuresgroup(int ID=-1);
   sift_featuresgroup(const sift_featuresgroup& fg);

// On 11/13/01, we learned from Tara Dennis that base class
// destructors ought to always be declared as virtual so that they
// will automatically be called by inherited class destructors:

   virtual ~sift_featuresgroup();
   sift_featuresgroup& operator= (const sift_featuresgroup& fg);
   friend std::ostream& operator<< (std::ostream& outstream,
                                    const sift_featuresgroup& fg);

// Set and get member functions:

   void add_siftfeature(sift_feature* sift_feature_ptr);
   int get_n_sift_features(int photo_ID) const;
   int count_all_sift_features() const;
   void write_SQL_insert_features_commands(
      int photo_ID,std::string SQL_filename) const;

  private:

   int ID;
   PHOTOSGROUP_SIFTFEATURES_MAP* photosgroup_siftfeatures_map_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const sift_featuresgroup& fg);

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

#endif  // sift_featuresgroup.h



