// ==========================================================================
// Header file for Pass class which holds imagery data from a single
// sensor collected at basically a single time.
// ==========================================================================
// Last updated on 9/7/08; 10/1/08; 3/8/09; 3/11/09; 12/11/09
// ==========================================================================

#ifndef PASS_H
#define PASS_H

#include <iostream>
#include <string>
#include <vector>
#include "passes/PassInfo.h"

class Pass
{

  public:

   enum PassType 
   {
      cloud, video, earth, surface_texture, GIS_layer, dataserver, 
      sensor_metadata, dted, other
   };

   enum InputFileType
   {
      xyzp,xyz,fxyz,xyzrgba,tdp,ive,osga,osg,vid,png,jpg,tif,rgb,ntf,
      dt0,dt1,dt2,pkg,unknown
   };

   Pass();
   Pass(int ID);
   ~Pass();
   friend std::ostream& operator<< (std::ostream& outstream,const Pass& p);

// Set & get methods:

   void set_PassInfo_ptr(PassInfo* PI_ptr);
   PassInfo* get_PassInfo_ptr();
   const PassInfo* get_PassInfo_ptr() const;

   void set_passtype(PassType t);
   const PassType& get_passtype() const;
   void set_input_filetype(InputFileType ift);
   Pass::InputFileType get_input_filetype() const;

   void pushback_filename(std::string input_filename);
   std::string get_first_filename() const;
   std::vector<std::string>& get_filenames();
   const std::vector<std::string>& get_filenames() const;

   std::string get_passname_prefix() const;

   void set_northern_hemisphere_flag(bool flag);
   bool get_northern_hemisphere_flag() const;
   void set_UTM_zonenumber(int zn);
   int get_UTM_zonenumber() const;

   int get_ID() const;

  private:

   PassInfo* PassInfo_ptr;
   PassType pass_type;
   InputFileType input_filetype;
   bool northern_hemisphere_flag;
   int UTM_zonenumber,ID;
   std::vector<std::string> filenames;

   void allocate_member_objects();
   void initialize_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void Pass::set_PassInfo_ptr(PassInfo* PI_ptr)
{
   PassInfo_ptr=PI_ptr;
}

inline PassInfo* Pass::get_PassInfo_ptr()
{
   return PassInfo_ptr;
}

inline const PassInfo* Pass::get_PassInfo_ptr() const
{
   return PassInfo_ptr;
}

inline void Pass::set_passtype(PassType t)
{
   pass_type=t;
}

inline const Pass::PassType& Pass::get_passtype() const
{
   return pass_type;
}

inline void Pass::set_input_filetype(InputFileType ift)
{
   input_filetype=ift;
}

inline Pass::InputFileType Pass::get_input_filetype() const
{
   return input_filetype;
}

inline void Pass::pushback_filename(std::string input_filename) 
{
   filenames.push_back(input_filename);
}

inline std::string Pass::get_first_filename() const
{
   if (filenames.size() > 0)
   {
      return filenames[0];
   }
   else
   {
      return "";
   }
}

inline std::vector<std::string>& Pass::get_filenames() 
{
   return filenames;
}

inline const std::vector<std::string>& Pass::get_filenames() const
{
   return filenames;
}

inline void Pass::set_northern_hemisphere_flag(bool flag)
{
   northern_hemisphere_flag=flag;
}

inline bool Pass::get_northern_hemisphere_flag() const
{
   return northern_hemisphere_flag;
}

inline void Pass::set_UTM_zonenumber(int zn)
{
//   std::cout << "**************************************************"
//             << std::endl;
//   std::cout << "inside Pass::set_UTM_zonenumber(), zone = " << zn
//             << std::endl;
//   std::cout << "**************************************************"
//             << std::endl;
   UTM_zonenumber=zn;
}

inline int Pass::get_UTM_zonenumber() const
{
   return UTM_zonenumber;
}

inline int Pass::get_ID() const
{
   return ID;
}

#endif // Pass.h

