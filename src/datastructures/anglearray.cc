// ==========================================================================
// Brian Kavanagh's routines for reading in projected area file
// written in RCS file format and interpolating projected area data
// ==========================================================================
// Last updated on 1/17/03
// ==========================================================================

#include "anglearray.h"

using std::string;
using std::ofstream;
using std::endl;

// ---------------------------------------------------------------------
// Constructor functions:
// ---------------------------------------------------------------------

anglearray::anglearray(void)
{
   const int NAZ_COLUMNS=2*721;
   const int NEL_ROWS=2*181;

   angle_value_array=new double[NEL_ROWS][NAZ_COLUMNS];

   az_num_steps=el_num_steps=delta_az=delta_el=0;
   az_first=az_last=el_first=el_last=0;
}

// When an object is initialized with an object of the same type, the
// following function is called.  This next constructor is apparently
// called whenever a function is passed an object as an argument:

anglearray::anglearray(const anglearray& a)
{
   docopy(a);
}

anglearray::~anglearray()
{
   delete [] angle_value_array;
   angle_value_array=NULL;
}

// ---------------------------------------------------------------------
// As Ed Broach has pointed out, the default C++ =operator for objects
// may simply equate pointers to arrays within objects when one object
// is equated with another.  Individual elements within the arrays
// apparently are not equated to one another by the default C++
// =operator.  This can lead to segmentation errors if the arrays are
// dynamically rather than statically allocated, for the pointer to
// the original array may be destroyed before the elements within the second
// array are copied over.  So we need to write an explicit copy function 
// which transfers all of the subfields within an object to another object
// whenever the object in question has dynamically allocated arrays rather 
// than relying upon C++'s default =operator:

void anglearray::docopy(const anglearray& a)
{
   const int NAZ_COLUMNS=2*721;
   const int NEL_ROWS=2*181;

   int i,j;

   az_num_steps=a.az_num_steps;
   el_num_steps=a.el_num_steps;
   delta_az=a.delta_az;
   delta_el=a.delta_el;
   az_first=a.az_first;
   az_last=a.az_last;
   el_first=a.el_first;
   el_last=a.el_last;
   
   for (i=0; i<NEL_ROWS; i++)
   {
      for (j=0; j<NAZ_COLUMNS; j++)
      {
         angle_value_array[i][j]=a.angle_value_array[i][j];
      }
   }
}

// Overload = operator:

anglearray& anglearray::operator= (const anglearray& a)
{
   docopy(a);
   return *this;
}

// ==========================================================================
// Subroutine read_rcs_file reads in the contents of a data file
// written in the standard Group 49 RCS file format.  It then loads
// the read in values into the anglearray object's two dimensional data
// array.

void anglearray::read_rcs_file(string filename)
{
   int i,nlines1,az_index,el_index;
   int test;
   double dummy;
   double az_2;         // Used to determine delta_az
   double el_2;         // Used to determine delta_el
   string line1[100000];
   string sign_nums( "+-1234567890" );

   ReadInfile(filename,line1,nlines1);

   i=0;
   az_index=el_index=0;

// Handle initial parameters and get to first "E"
   while ((test = line1[i].find("E") == string::npos) && (i < nlines1))
      i++;

// Get past the first "E"
   i++;

// Get the first el, the first two az values, and the last az value
   el_first = string_to_number(line1[i]);
   i++;
   string_to_two_numbers(line1[i],az_first,
                         angle_value_array[el_index][az_index]);
   i++;
   az_index++;
   string_to_two_numbers(line1[i],az_2,
                         angle_value_array[el_index][az_index]);
   i++;
   az_index++;

   while ((test = line1[i].find("E") == string::npos) && (i < nlines1))
   {
     // Ignore lines without numbers or an "E" (blank lines)
     if (line1[i].find_first_of( sign_nums, 0) == -1) 
	  i++;
     else
       {
	 string_to_two_numbers(line1[i],az_last,
			       angle_value_array[el_index][az_index]);
	 i++;
	 az_index++;
       }
   }
   el_index++;

   i++;                   // Get past the "E" and back to function values

// Get the second el
   if(i < nlines1)
   {
     el_2 = string_to_number(line1[i]);
     el_last = el_2; // In case there are only two elevation cuts
     i++;

     az_index = 0;
     string_to_two_numbers(line1[i],dummy,
			   angle_value_array[el_index][az_index]);
     i++;
     az_index++;
     string_to_two_numbers(line1[i],dummy,
			   angle_value_array[el_index][az_index]);
     i++;
     az_index++;
     while ((test = line1[i].find("E") == string::npos) && (i < nlines1))
       {
	 // Ignore lines without numbers or an "E" (blank lines)
	 if (line1[i].find_first_of( sign_nums, 0) == -1) 
	   i++;
	 else
	   {
	     string_to_two_numbers(line1[i],dummy,
				   angle_value_array[el_index][az_index]);
	     i++;
	     az_index++;
	   }
       }
     el_index++;
 
     delta_az = az_2 - az_first;
     delta_el = el_2 - el_first;
     az_num_steps = int((az_last - az_first)/delta_az + 1);

   }   
   else // Handle single elevation case
   {
     delta_az = az_2 - az_first;
     delta_el = 0.0;
     az_num_steps = int((az_last - az_first)/delta_az + 1);
   }      

// Finally start the loop to get the rest of the function values, and
// the last el value 
   while (i < nlines1)
   { 
     i++;                                       // Move past "E" line        
     el_last = string_to_number(line1[i]);
     i++;                                       // Move past "el" line
     az_index = 0;
     while ((test = line1[i].find("E") == string::npos) && (i < nlines1))
     {
       // Ignore lines without numbers or an "E" (blank lines)
       if (line1[i].find_first_of( sign_nums, 0) == -1) 
	 i++;
       else
       {
	 string_to_two_numbers(line1[i],dummy,
			       angle_value_array[el_index][az_index]);
	 i++;
	 az_index++;
       }
     }    
     el_index++;
   }

// Determine the number of elevations in the file
   if (delta_el == 0.0)
     el_num_steps = 1;
   else
     el_num_steps = int((el_last - el_first)/delta_el + 1);
}

// ==========================================================================
// Subroutine write_rcs_file writes out the contents of the anglearray
// object's 2D data array to an output file in the standard Group 49
// RCS file format

void anglearray::write_rcs_file(string filename)
{
   int i,j;
   double currel,curraz;
   ofstream outfile;

   openfile(filename,outfile);

   for (j=0; j<el_num_steps; j++)
   {
      outfile << "E" << endl;
      currel=el_first+j*delta_el;
      outfile << currel << endl;
      for (i=0; i<az_num_steps; i++)
      {
         curraz=az_first+i*delta_az;
         outfile << curraz << "\t" << angle_value_array[j][i] << endl;
      }
   }
   outfile.close();
}

// ==========================================================================
// Given an input azimuth and elevation specified in degrees,
// subroutine interpangles performs a 2D linear interpolation of the
// values stored within the array angle_value_array and returns the
// interpolated value.  The periodicity of azimuth values is taken
// into account by this routine.

double anglearray::interpangles(double az_input,double el_input)
{

// Indices for the four points we'll use in our interpolation
   
   int az_used_1,az_used_2,el_used_1,el_used_2;
   double az_diff,el_diff;
   double value;        // Our final interpolated value
     
// Handle el being below range, above range, or in range of known values

   if (el_num_steps == 1)
   {
      el_used_1 = 0;
      el_used_2 = 0;
      el_diff = 0;
   } 
   else if ((el_input < el_first && delta_el > 0.0) 
            || (el_input > el_first && delta_el < 0.0))
   {
      el_used_1 = 0;
      el_used_2 = 0;
      el_diff = 0;
   }
   else if ((el_input > el_last && delta_el > 0.0) 
            || (el_input < el_last && delta_el < 0.0))
   {
      el_used_1 = el_num_steps -1;
      el_used_2 = el_num_steps -1;
      el_diff = 0;
   }
   else
   {
      el_used_1 = (int)((el_input - el_first)/delta_el);
      el_used_2 = el_used_1 + 1;
      el_diff = (el_first + delta_el*el_used_2 
                    - el_input)/delta_el;
   }
	   
// Handle az being below range, above range, or in range of known values
   if (az_input < az_first)
     while (az_input < az_first)
       az_input = az_input + 360.0;
   else if (az_input > az_last)
     while (az_input > az_last)
       az_input = az_input - 360.0;

   az_used_1 = (int)((az_input - az_first)/delta_az);
   az_used_2 = az_used_1 + 1;
   az_diff = az_used_2 - az_input/delta_az;
   az_diff = (az_first + delta_az*az_used_2 - az_input)/delta_az;

// Interpolate linearly to determine  approximate value 

   value = el_diff*az_diff*angle_value_array[el_used_1][az_used_1]
      +el_diff*(1-az_diff)*angle_value_array[el_used_1][az_used_2]
      +(1-el_diff)*az_diff*angle_value_array[el_used_2][az_used_1]
      +(1-el_diff)*(1-az_diff)*angle_value_array[el_used_2][az_used_2];
   return value;
}

