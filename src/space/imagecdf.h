// ==========================================================================
// Header file for imagecdf class
// ==========================================================================
// Last modified on 4/27/06; 7/18/06; 7/20/06; 8/22/06
// ==========================================================================

#ifndef IMAGECDF_H
#define IMAGECDF_H

#include <string>
#include <vector>
#include "space/motionfuncs.h"

class ground_radar;
class satelliteimage;
class satellitepass;
//class sensor;

class imagecdf
{

  public:

   imagecdf(std::string sat_name,satellitepass* satpass_ptr);
   imagecdf(const imagecdf& i);
   ~imagecdf();
   imagecdf& operator= (const imagecdf& i);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const imagecdf& i);

// Set and get methods:

   void set_filename(std::string name);
   int get_nimages() const;
   int get_pass_date() const;
   std::string get_passname() const;
   std::string get_targetname() const;
   motionfunc::Imagery_motion_type get_imagery_motion_type() const;

// Parsing member functions:

   void select_file(std::string input_filename);
   void select_file(
      bool input_param_file,std::string inputline[],
      unsigned int& currlinenumber);
   bool readin_file(bool regularize_images);
   void readin_headerinfo();
   void readin_data();
   void writeout_file(std::string filename_descriptor="new");

  private: 

   bool unix_compressed_flag;
   int nc_id;	// netcdf file ID
   int number_of_images;
   int pass_date; // Pass date.  Measured in integer number of seconds since 
		  // 1970-01-01 (Julian date = 2440587.5) till midnight of 
		  // pass collection day.

   motionfunc::Imagery_motion_type imagery_motion_type;
   
   std::string filename,passname,target_name;
   std::string xeliasfile_type_str,object_number_str;
   std::string starting_date_and_midtime_str,ending_date_and_midtime_str;
//   sensor* sensor_ptr;
   ground_radar* ground_radar_ptr;

// Each imagecdfz possesses a pointer back to the satellitepass with
// which it is associated:

   satellitepass* pass_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const imagecdf& i);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void imagecdf::set_filename(std::string name)
{
   filename=name;
}

inline int imagecdf::get_nimages() const
{
   return number_of_images;
}

inline int imagecdf::get_pass_date() const
{
   return pass_date;
}

inline std::string imagecdf::get_passname() const
{
   return passname;
}

inline std::string imagecdf::get_targetname() const
{
   return target_name;
}

inline motionfunc::Imagery_motion_type imagecdf::get_imagery_motion_type() 
   const
{
   return imagery_motion_type;
}



#endif // imagecdf.h
