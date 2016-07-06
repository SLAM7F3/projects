/********************************************************************
 *
 * Name: xyz_file.cc
 *
 *
 * Author: Joseph Adams
 *
 * Description:
 * xyzp file class
 *
 **********************************************************************/

#include "xyzp_file.h"

// default constructor;

xyzp_file::xyzp_file(){}

xyzp_file::xyzp_file(
   const char* filename, int ts=sizeof(float)):base_file(filename)
{
   header_size=0;
   default_poffset=0;
   stride=4;
   type_size=ts;
   updir=2;
   calculate_sizes();
}

void xyzp_file::calculate_sizes()
{
   open_input_read_only_binary();
   data_size=file_size()-header_size;
   n_data_points=data_size/type_size/stride;
   close();
}

float* xyzp_file::read_data(long max_points)
{
   open_input_read_only_binary();
   move_to_beginning();
   long read_data_size=data_size;
   long read_n_data_points=n_data_points;
   if (max_points>0 && n_data_points>max_points)
   {
      read_data_size=max_points*stride*type_size;
      read_n_data_points=max_points;
   }
   float* data = (float *)malloc(read_data_size);
   if (data!=NULL)
   {
      printf("Allocated %d bytes for data\n",read_data_size);
   }
   else
   {
      exit(1);
   }
   move_to_beginning();    
   read_binary(data,type_size,read_data_size/type_size, fp);
	
   if (type_size==sizeof(double))
   {
      // convert to float
      double* ddata=(double*) data;

      for (int i=0; i<read_n_data_points*stride; i++)
      {
         data[i]=(float)ddata[i];
      }
      data=(float*)realloc(data,read_data_size/sizeof(double)*sizeof(float));
   }

   close();

/*
// Experiment attempted on 6/8/04 to ignore xyzp quadruples in which
// either z and/or p equals NEGATIVEINFINITY:

   for (int n=0; n<read_n_data_points; n++)
   {
      bool point_displayable=true;
      for (int i=0; i<stride; i++)
      {
         if (data[n*stride+i] < -1000000)
         {
            point_displayable=false;
         }
      }
      if (!point_displayable)
      {
         for (int i=0; i<stride; i++)
         {
            data[n*stride+i]=0;
         }
      }
   }
*/
 
   return data;
}

long xyzp_file::getNumberOfPoints()
{
   return n_data_points;
}

int xyzp_file::getStride()
{
   return stride;
}

void xyzp_file::setStride(int stride_)
{
   stride=stride_;
   calculate_sizes();
}

void xyzp_file::move_to_data_beginning(){
   move_to(header_size);
}

int xyzp_file::getDefaultPOffset()
{
   return default_poffset;
}

int xyzp_file::getUpDirection()
{
   return updir;
}

void xyzp_file::getViewPoint(float &x, float &y, float &z)
{
   x=y=z=0.0;
}
