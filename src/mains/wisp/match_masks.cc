// ========================================================================
// Program MATCH_MASKS reads in two cylindrically projected range
// masks corresponding to consecutive 36-degree WISP panels.  The
// angular extents of cylindrical projections overlap.  So MATCH_MASKS
// attempts to find the pixel columns within the nth and n+1st masks
// which correspond.  

// For 999x640 masks, MATCH_MASKS indicates that column 976 in the nth
// mask matches onto column 10 in the n+1st mask.

// ========================================================================
// Last updated on 12/26/11
// ========================================================================

#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "image/raster_parser.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::pair;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

   vector<string> mask_filenames;
   mask_filenames.push_back("mask_R_7.jpg");
   mask_filenames.push_back("mask_R_8.jpg");
//   mask_filenames.push_back("mask_R_9.jpg");
//   mask_filenames.push_back("mask_R_0.jpg");
//   mask_filenames.push_back("mask_R_1.jpg");
//   mask_filenames.push_back("mask_R_2.jpg");
//   mask_filenames.push_back("mask_R_3.jpg");
//   mask_filenames.push_back("mask_R_4.jpg");
//   mask_filenames.push_back("mask_R_5.jpg");
//   mask_filenames.push_back("mask_R_6.jpg");
//   mask_filenames.push_back("mask_R_7.jpg");
   
   vector<raster_parser*> RasterParser_ptrs;
   vector<twoDarray*> RtwoDarray_ptrs,GtwoDarray_ptrs,BtwoDarray_ptrs;

   vector<int> mdims,ndims;
   
   for (unsigned int i=0; i<mask_filenames.size(); i++)
   {
      raster_parser* RasterParser_ptr=new raster_parser;
      RasterParser_ptrs.push_back(RasterParser_ptr);
      
      RasterParser_ptr->open_image_file(mask_filenames[i]);
      int n_channels=RasterParser_ptr->get_n_channels();
      cout << "n_channels = " << n_channels << endl;
      if (n_channels != 3) continue;
      
      for (int channel_ID=0; channel_ID<n_channels; channel_ID++)
      {
         RasterParser_ptr->fetch_raster_band(channel_ID);

         cout << "channel_ID = " << channel_ID << endl;
         if (channel_ID==0)
         {
            RtwoDarray_ptrs.push_back(RasterParser_ptr->get_RtwoDarray_ptr());
            mdims.push_back(RtwoDarray_ptrs.back()->get_mdim());
            ndims.push_back(RtwoDarray_ptrs.back()->get_ndim());
            
//            cout << "RtwoDarray_ptr = " << RtwoDarray_ptr << endl;
            RasterParser_ptr->read_raster_data(RtwoDarray_ptrs.back());
         }
         else if (channel_ID==1)
         {
            GtwoDarray_ptrs.push_back(RasterParser_ptr->get_GtwoDarray_ptr());
//            cout << "GtwoDarray_ptr = " << GtwoDarray_ptr << endl;
            RasterParser_ptr->read_raster_data(GtwoDarray_ptrs.back());
         }
         else if (channel_ID==2)
         {
            BtwoDarray_ptrs.push_back(RasterParser_ptr->get_BtwoDarray_ptr());
//            cout << "BtwoDarray_ptr = " << BtwoDarray_ptr << endl;
            RasterParser_ptr->read_raster_data(BtwoDarray_ptrs.back());
         }
      } // loop over channel_ID labeling RGB channels

      RasterParser_ptr->close_image_file();
   } // loop over index i 

// Store first column of RGB values from second mask:

   vector<int> R2,G2,B2;
//   int px_start=0;
   int px_start=10;
   for (int py=0; py<ndims[1]; py++)
   {
      R2.push_back(RtwoDarray_ptrs[1]->get(px_start,py));
      G2.push_back(GtwoDarray_ptrs[1]->get(px_start,py));
      B2.push_back(BtwoDarray_ptrs[1]->get(px_start,py));
   } // loop over py index

   int best_px=-1;
   double min_error=POSITIVEINFINITY;
   for (int px=mdims[0]-1; px > mdims[0]-100; px--)
   {
      double error=0;
      for (int py=0; py<ndims[0]; py++)
      {
         double dR=RtwoDarray_ptrs[0]->get(px,py)-R2[py];
         double dG=GtwoDarray_ptrs[0]->get(px,py)-G2[py];
         double dB=BtwoDarray_ptrs[0]->get(px,py)-B2[py];

//         cout << "py = " << py << " dR = " << dR
//              << " dG = " << dG << " dB = " << dB << endl;
         
         error += fabs(dR)+fabs(dG)+fabs(dB);
      } // loop over py index
      error /= ndims[0];

      if (error < min_error)
      {
         min_error=error;
         best_px=px;
         cout << "min_error = " << min_error << " best_px = " << best_px
              << endl;
      }
   } // loop over px index

}


