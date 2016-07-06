// ==========================================================================
// Mserfuncs namespace method definitions
// ==========================================================================
// Last modified on 10/8/12; 10/15/12; 6/7/14
// ==========================================================================

#include <iostream>

#include "math/basic_math.h"
#include "color/colorfuncs.h"
#include "image/extremal_region.h"
#include "general/filefuncs.h"
#include "image/graphicsfuncs.h"
#include "image/imagefuncs.h"
#include "video/mserfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "datastructures/union_find.h"

using std::cerr;
using std::cout;
using std::endl;
using std::map;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

namespace mserfunc
{

   int dark_ID_offset=10000;
   EXTREMAL_REGIONS_MAP::iterator iter;

// ==========================================================================
// MSER methods
// ==========================================================================

// Method purge_regions_map() iterates through input
// *extremal_region_map_ptr and destroys each extremal region entry.
// It also clears the STL map.

      void purge_regions_map(
         EXTREMAL_REGIONS_MAP* extremal_region_map_ptr)
      {
         cout << "inside mserfunc::purge_regions_map()" << endl;

         if (extremal_region_map_ptr==NULL) return;

         for (iter=extremal_region_map_ptr->begin(); 
              iter != extremal_region_map_ptr->end(); iter++)
         {
            delete iter->second;
         }
         extremal_region_map_ptr->clear();
      }

// ---------------------------------------------------------------------
// Method extract_MSERs() takes in an image and calls the Oxford MSER
// linux binary which outputs RLE runs for locally dark and bright
// maximally stable extremal regions.  This method decodes the MSER
// linux binary output and instantiates extremal region objects which
// are returned in STL maps.

      void extract_MSERs(
         string image_filename,
         EXTREMAL_REGIONS_MAP* dark_extremal_region_map_ptr,
         EXTREMAL_REGIONS_MAP* bright_extremal_region_map_ptr)
      {
         cout << "inside mserfunc::extract_MSERs()" << endl;

// Next call Oxford MSER linux binary on input image file:

         unsigned int xdim,ydim;
         imagefunc::get_image_width_height(image_filename,xdim,ydim);
//         cout << "xdim = " << xdim << " ydim = " << ydim << endl;

         string MSER_filename="./mser.output";
         string unix_cmd="mser.ln -i "+image_filename+" -o "+MSER_filename;
         sysfunc::unix_command(unix_cmd);
         filefunc::ReadInfile(MSER_filename);

         int line_counter=0;
         int n_dark_regions=stringfunc::string_to_number(
            filefunc::text_line[line_counter++]);
         cout << "n_dark_regions = " << n_dark_regions << endl;

         int region_ID=1;
         vector<int> RLE_pixel_IDs;

         for (int i=1; i<=n_dark_regions; i++)
         {
            RLE_pixel_IDs.clear();
            vector<double> column_values=stringfunc::string_to_numbers(
               filefunc::text_line[i]);
            line_counter++;

            int n_triples=column_values[0];
            for (int n=0; n<n_triples; n++)
            {
               int row=column_values[3*n+1];
               int start_column=column_values[3*n+2];
               int stop_column=column_values[3*n+3];

//         cout << "row = " << row << " start_column = " << start_column
//              << " stop_column = " << stop_column << endl;

               int start_pixel_ID=graphicsfunc::get_pixel_ID(
                  start_column,row,xdim);
               int stop_pixel_ID=graphicsfunc::get_pixel_ID(
                  stop_column,row,xdim);
               RLE_pixel_IDs.push_back(start_pixel_ID);
               RLE_pixel_IDs.push_back(stop_pixel_ID);

               if (start_pixel_ID > stop_pixel_ID ||
               start_pixel_ID < 0 || stop_pixel_ID < 0)
               {
                  cout << "Error!  start_pixel_ID = " << start_pixel_ID
                       << " stop_pixel_ID = " << stop_pixel_ID << endl;
                  outputfunc::enter_continue_char();
               }
            }
  
            extremal_region* extremal_region_ptr=new extremal_region(
               region_ID++);
            extremal_region_ptr->set_RLE_pixel_IDs(RLE_pixel_IDs);
            (*dark_extremal_region_map_ptr)[extremal_region_ptr->get_ID()]=
               extremal_region_ptr;
         } // loop over index i labeling dark MSERs

         RLE_pixel_IDs.clear();

         int n_bright_regions=stringfunc::string_to_number(
            filefunc::text_line[line_counter++]);
         cout << "n_bright_regions = " << n_bright_regions << endl;

         for (int i=n_dark_regions+2; i<n_dark_regions+2+n_bright_regions; i++)
         {
            RLE_pixel_IDs.clear();
            vector<double> column_values=stringfunc::string_to_numbers(
               filefunc::text_line[i]);
            line_counter++;

            int n_triples=column_values[0];
            for (int n=0; n<n_triples; n++)
            {
               int row=column_values[3*n+1];
               int start_column=column_values[3*n+2];
               int stop_column=column_values[3*n+3];

//         cout << "row = " << row << " start_column = " << start_column
//              << " stop_column = " << stop_column << endl;

               int start_pixel_ID=graphicsfunc::get_pixel_ID(
                  start_column,row,xdim);
               int stop_pixel_ID=graphicsfunc::get_pixel_ID(
                  stop_column,row,xdim);
               RLE_pixel_IDs.push_back(start_pixel_ID);
               RLE_pixel_IDs.push_back(stop_pixel_ID);

               if (start_pixel_ID > stop_pixel_ID ||
               start_pixel_ID < 0 || stop_pixel_ID < 0)
               {
                  cout << "Error!  start_pixel_ID = " << start_pixel_ID
                       << " stop_pixel_ID = " << stop_pixel_ID << endl;
                  outputfunc::enter_continue_char();
               }
            } // loop over index n labeling RLE triples
  
            extremal_region* extremal_region_ptr=
               new extremal_region(region_ID++);
            extremal_region_ptr->set_RLE_pixel_IDs(RLE_pixel_IDs);
            (*bright_extremal_region_map_ptr)[extremal_region_ptr->get_ID()]=
               extremal_region_ptr;

         } // loop over index i labeling dark MSERs

         cout << "dark_extremal_region_map_ptr->size() = "
              << dark_extremal_region_map_ptr->size() << endl;
         cout << "bright_extremal_region_map_ptr->size() = "
              << bright_extremal_region_map_ptr->size() << endl;
      }

// ---------------------------------------------------------------------
// Method extract_MSERs() takes in an image and calls the Oxford MSER
// linux binary which outputs RLE runs for locally dark and bright
// maximally stable extremal regions.  This method decodes the MSER
// linux binary output and instantiates extremal region objects which
// are returned in STL vectors.

      void extract_MSERs(
         string image_filename,
         vector<extremal_region*>& dark_extremal_region_ptrs,
         vector<extremal_region*>& bright_extremal_region_ptrs)
      {
         cout << "inside mserfunc::extract_MSERs()" << endl;

// First purge any entries in dark[bright]_extremal_region_ptrs:

         for (unsigned int d=0; d<dark_extremal_region_ptrs.size(); d++)
         {
            delete dark_extremal_region_ptrs[d];
         }
         dark_extremal_region_ptrs.clear();

         for (unsigned int b=0; b<bright_extremal_region_ptrs.size(); b++)
         {
            delete bright_extremal_region_ptrs[b];
         }
         bright_extremal_region_ptrs.clear();
         
         unsigned int xdim,ydim;
         imagefunc::get_image_width_height(image_filename,xdim,ydim);
//         cout << "xdim = " << xdim << " ydim = " << ydim << endl;

         string MSER_filename="./mser.output";
         string unix_cmd="mser.ln -i "+image_filename+" -o "+MSER_filename;
         sysfunc::unix_command(unix_cmd);
         filefunc::ReadInfile(MSER_filename);

         int line_counter=0;
         int n_dark_regions=stringfunc::string_to_number(
            filefunc::text_line[line_counter++]);
         cout << "n_dark_regions = " << n_dark_regions << endl;

         int region_ID=1;
         vector<int> RLE_pixel_IDs;

         for (int i=1; i<=n_dark_regions; i++)
         {
            RLE_pixel_IDs.clear();
            vector<double> column_values=stringfunc::string_to_numbers(
               filefunc::text_line[i]);
            line_counter++;

            int n_triples=column_values[0];
            for (int n=0; n<n_triples; n++)
            {
               int row=column_values[3*n+1];
               int start_column=column_values[3*n+2];
               int stop_column=column_values[3*n+3];

//         cout << "row = " << row << " start_column = " << start_column
//              << " stop_column = " << stop_column << endl;

               int start_pixel_ID=graphicsfunc::get_pixel_ID(
                  start_column,row,xdim);
               int stop_pixel_ID=graphicsfunc::get_pixel_ID(
                  stop_column,row,xdim);
               RLE_pixel_IDs.push_back(start_pixel_ID);
               RLE_pixel_IDs.push_back(stop_pixel_ID);

               if (start_pixel_ID > stop_pixel_ID ||
               start_pixel_ID < 0 || stop_pixel_ID < 0)
               {
                  cout << "Error!  start_pixel_ID = " << start_pixel_ID
                       << " stop_pixel_ID = " << stop_pixel_ID << endl;
                  outputfunc::enter_continue_char();
               }
            }
  
            extremal_region* extremal_region_ptr=new extremal_region(
               region_ID++);
            extremal_region_ptr->set_RLE_pixel_IDs(RLE_pixel_IDs);
            dark_extremal_region_ptrs.push_back(extremal_region_ptr);
         } // loop over index i labeling dark MSERs


         RLE_pixel_IDs.clear();

         int n_bright_regions=stringfunc::string_to_number(
            filefunc::text_line[line_counter++]);
         cout << "n_bright_regions = " << n_bright_regions << endl;

         for (int i=n_dark_regions+2; i<n_dark_regions+2+n_bright_regions; i++)
         {
            RLE_pixel_IDs.clear();
            vector<double> column_values=stringfunc::string_to_numbers(
               filefunc::text_line[i]);
            line_counter++;

            int n_triples=column_values[0];
            for (int n=0; n<n_triples; n++)
            {
               int row=column_values[3*n+1];
               int start_column=column_values[3*n+2];
               int stop_column=column_values[3*n+3];

//         cout << "row = " << row << " start_column = " << start_column
//              << " stop_column = " << stop_column << endl;

               int start_pixel_ID=graphicsfunc::get_pixel_ID(
                  start_column,row,xdim);
               int stop_pixel_ID=graphicsfunc::get_pixel_ID(
                  stop_column,row,xdim);
               RLE_pixel_IDs.push_back(start_pixel_ID);
               RLE_pixel_IDs.push_back(stop_pixel_ID);

               if (start_pixel_ID > stop_pixel_ID ||
               start_pixel_ID < 0 || stop_pixel_ID < 0)
               {
                  cout << "Error!  start_pixel_ID = " << start_pixel_ID
                       << " stop_pixel_ID = " << stop_pixel_ID << endl;
                  outputfunc::enter_continue_char();
               }
            } // loop over index n labeling RLE triples
  
            extremal_region* extremal_region_ptr=
               new extremal_region(region_ID++);
            extremal_region_ptr->set_RLE_pixel_IDs(RLE_pixel_IDs);
            bright_extremal_region_ptrs.push_back(extremal_region_ptr);
         } // loop over index i labeling dark MSERs

//         cout << "dark_extremal_region_ptrs.size() = "
//              << dark_extremal_region_ptrs.size() << endl;
//         cout << "bright_extremal_region_ptrs.size() = "
//              << bright_extremal_region_ptrs.size() << endl;
      }

// ---------------------------------------------------------------------
// Method update_MSER_twoDarray() takes in STL vectors containing
// locally bright and dark maximally stable extremal regions.  It sets
// bright [dark] MSER pixels equal to region_ID [region_ID+dark_ID_offset]
// within the ptwoDarray member of *mser_texture_rectangle_ptr.

      void update_MSER_twoDarray(
         const vector<extremal_region*>& bright_extremal_region_ptrs,
         texture_rectangle* mser_texture_rectangle_ptr)
     {
//         cout << "inside mserfunc::update_MSER_twoDarray()" << endl;
        vector<extremal_region*> dark_extremal_region_ptrs;

         cout << "bright_extremal_region_ptrs.size() = "
              << bright_extremal_region_ptrs.size() << endl;

        update_MSER_twoDarray(
           dark_extremal_region_ptrs,bright_extremal_region_ptrs,
           mser_texture_rectangle_ptr);
     }
      
      void update_MSER_twoDarray(
         const vector<extremal_region*>& dark_extremal_region_ptrs,
         const vector<extremal_region*>& bright_extremal_region_ptrs,
         texture_rectangle* mser_texture_rectangle_ptr)
     {
        cout << "inside mserfunc::update_MSER_twoDarray()" << endl;

         mser_texture_rectangle_ptr->instantiate_ptwoDarray_ptr();
         twoDarray* mser_twoDarray_ptr=mser_texture_rectangle_ptr->
            get_ptwoDarray_ptr();
         mser_twoDarray_ptr->clear_values();

         for (unsigned int e=0; e<bright_extremal_region_ptrs.size(); e++)
         {
            int bright_ID=bright_extremal_region_ptrs[e]->get_ID();
//            cout << "e = " << e << " bright_ID = " << bright_ID << endl;
            bright_extremal_region_ptrs[e]->run_length_decode(
               mser_twoDarray_ptr,bright_ID);
         }

         for (unsigned int e=0; e<dark_extremal_region_ptrs.size(); e++)
         {
            int dark_ID=dark_extremal_region_ptrs[e]->get_ID();
//            cout << "e = " << e << " dark_ID = " << dark_ID << endl;
            dark_extremal_region_ptrs[e]->run_length_decode(
               mser_twoDarray_ptr,dark_ID+dark_ID_offset);      
         }

/*
         for (int py=0; py<mser_twoDarray_ptr->get_ydim(); py++)
         {
            for (int px=0; px<mser_twoDarray_ptr->get_xdim(); px++)
            {
               int cc_ID=mser_twoDarray_ptr->get(px,py);
               if (cc_ID > 0)
               {
                  cout << "px = " << px << " py = " << py 
                       << " cc_ID = " << cc_ID << endl;
               }
            } // loop over px
         } // loop over py
         outputfunc::enter_continue_char();
*/

      }

// ---------------------------------------------------------------------      
      void update_MSER_twoDarray(
         EXTREMAL_REGIONS_MAP* bright_extremal_region_map_ptr,
         texture_rectangle* mser_texture_rectangle_ptr)
     {
        cout << "inside mserfunc::update_MSER_twoDarray() #3" << endl;
        cout << "bright_extremal_region_map_ptr->size() = "
             << bright_extremal_region_map_ptr->size() << endl;
        EXTREMAL_REGIONS_MAP* dark_extremal_region_map_ptr=NULL;
        update_MSER_twoDarray(
           dark_extremal_region_map_ptr,
           bright_extremal_region_map_ptr,mser_texture_rectangle_ptr);
     }
      
      void update_MSER_twoDarray(
         EXTREMAL_REGIONS_MAP* dark_extremal_region_map_ptr,
         EXTREMAL_REGIONS_MAP* bright_extremal_region_map_ptr,
         texture_rectangle* mser_texture_rectangle_ptr)
     {
        cout << "inside mserfunc::update_MSER_twoDarray() #4" << endl;

         mser_texture_rectangle_ptr->instantiate_ptwoDarray_ptr();
         twoDarray* mser_twoDarray_ptr=mser_texture_rectangle_ptr->
            get_ptwoDarray_ptr();
         mser_twoDarray_ptr->clear_values();

//         int e=0;
         for (iter=bright_extremal_region_map_ptr->begin();
              iter != bright_extremal_region_map_ptr->end(); iter++)
         {
            int bright_ID=iter->first;
//            cout << "e = " << e++ << " bright_ID = " << bright_ID << endl;
            iter->second->run_length_decode(mser_twoDarray_ptr,bright_ID);
         }

         if (dark_extremal_region_map_ptr != NULL)
         {
//            e=0;
            for (iter=dark_extremal_region_map_ptr->begin();
                 iter != dark_extremal_region_map_ptr->end(); iter++)
            {
               int dark_ID=iter->first;
//               cout << "e = " << e++ << " dark_ID = " << dark_ID << endl;
               iter->second->run_length_decode(mser_twoDarray_ptr,
               dark_ID+dark_ID_offset);
            }
         }

/*
         for (int py=0; py<mser_twoDarray_ptr->get_ydim(); py++)
         {
            for (int px=0; px<mser_twoDarray_ptr->get_xdim(); px++)
            {
               int cc_ID=mser_twoDarray_ptr->get(px,py);
               if (cc_ID > 0)
               {
                  cout << "px = " << px << " py = " << py 
                       << " cc_ID = " << cc_ID << endl;
               }
            } // loop over px
         } // loop over py
         outputfunc::enter_continue_char();
*/

      }

// ---------------------------------------------------------------------
// Method draw_MSERs() locally bright [dark] pixels as red [blue] in
// in *mser_texture_rectangle_ptr.

      void draw_MSERs(twoDarray* cc_twoDarray_ptr,
      texture_rectangle* mser_texture_rectangle_ptr)
      {
//         cout << "inside mserfunc::draw_MSERs()" << endl;
//         cout << "cc_twoDarray_ptr = " << cc_twoDarray_ptr << endl;
         const int dark_ID_offset=10000;

         for (unsigned int py=0; py<cc_twoDarray_ptr->get_ydim(); py++)
         {
            for (unsigned int px=0; px<cc_twoDarray_ptr->get_xdim(); px++)
            {
               int R,G,B;
               R=G=B=0;

               int cc_ID=cc_twoDarray_ptr->get(px,py);
//               cout << "px = " << px << " py = " << py 
//                    << " cc_ID = " << cc_ID << endl;

               if (cc_ID > dark_ID_offset)
               {
                  R=0;
                  G=128;
                  B=255;
               }
               else if (cc_ID >= 1)
               {
                  colorfunc::Color curr_color=colorfunc::get_color(cc_ID%10);
                  colorfunc::RGB curr_RGB=colorfunc::get_RGB_values(
                     curr_color);
                  R=curr_RGB.first*255;
                  G=curr_RGB.second*255;
                  B=curr_RGB.third*255;
//                  R=255;
//                  G=B=0;
               }
               mser_texture_rectangle_ptr->set_pixel_RGB_values(px,py,R,G,B);
               
            } // loop over px index
         } // loop over py index
      }

// ==========================================================================
// MSER border methods
// ==========================================================================
      
// Method identify_border_pixels() locates border pixels around each
// locally bright MSER & stores their extremal region IDs within
// *cc_borders_twoDarray_ptr:

      void identify_border_pixels(
         int border_thickness,twoDarray* tmp_twoDarray_ptr,
         EXTREMAL_REGIONS_MAP* extremal_region_map_ptr,
         twoDarray* cc_borders_twoDarray_ptr)
      {
         cout << "inside mserfunc::identify_border_pixels()" << endl;

         cc_borders_twoDarray_ptr->initialize_values(-1);

         for (iter=extremal_region_map_ptr->begin();
              iter != extremal_region_map_ptr->end(); iter++)
         {
            extremal_region* extremal_region_ptr=iter->second;
            vector<pair<int,int> > border_pixels=
               extremal_region_ptr->compute_border_pixels(
                  border_thickness,tmp_twoDarray_ptr);
            for (unsigned int bp=0; bp<border_pixels.size(); bp++)
            {
               int px=border_pixels[bp].first;
               int py=border_pixels[bp].second;
               cc_borders_twoDarray_ptr->put(
                  px,py,extremal_region_ptr->get_ID());
            } // loop over index bp
//            cout << "border_pixels.size() = " << border_pixels.size()
//                 << endl;
         } // iteration loop over extremal regions
         
      }

// ---------------------------------------------------------------------
// Method coalesce_touching_regions() takes in *cc_twoDarray_ptr which
// is assumed to be filled with connected component IDs > 0. We first 
// instantiate a UnionFind datastructure with the IDs for all extremal
// regions within input *extremal_region_map_ptr.  Looping over all
// pixels in *cc_twoDarray_ptr, we next associate pairs of extremal
// regions if the border region of one contains pixels that lie inside
// of the other. This method returns a compactified extremal_regions
// map which effectively contains the union of all the input extremal
// regions.

      EXTREMAL_REGIONS_MAP* coalesce_touching_regions(
         twoDarray* tmp_twoDarray_ptr,twoDarray* cc_twoDarray_ptr,
         EXTREMAL_REGIONS_MAP* extremal_region_map_ptr)
      {
         cout << "inside mserfunc::coalesce_touching_regions()" << endl;
         
         union_find uf;
         for (iter=extremal_region_map_ptr->begin();
              iter != extremal_region_map_ptr->end(); iter++)
         {
            extremal_region* extremal_region_ptr=iter->second;
            uf.MakeSet(extremal_region_ptr->get_ID());
         }

// Link any two extremal regions which have neighboring pixels:

         int border_thickness=1;
         for (iter=extremal_region_map_ptr->begin();
              iter != extremal_region_map_ptr->end(); iter++)
         {
            extremal_region* extremal_region_ptr=iter->second;
            int curr_ID=extremal_region_ptr->get_ID();
            
            vector<pair<int,int> > border_pixels=
               extremal_region_ptr->compute_border_pixels(
                  border_thickness,tmp_twoDarray_ptr);
            for (unsigned int bp=0; bp<border_pixels.size(); bp++)
            {
               int px=border_pixels[bp].first;
               int py=border_pixels[bp].second;
               int cc_ID=cc_twoDarray_ptr->get(px,py);
               if (cc_ID <= 0) continue;
               uf.Link(curr_ID,cc_ID);
            } // loop over bp
         } // iteration loop over extremal regions

         EXTREMAL_REGIONS_MAP* coalesced_extremal_region_map_ptr=
            new EXTREMAL_REGIONS_MAP;

         tmp_twoDarray_ptr->clear_values();
         for (unsigned int py=0; py<tmp_twoDarray_ptr->get_ydim(); py++)
         {
            for (unsigned int px=0; px<tmp_twoDarray_ptr->get_xdim(); px++)
            {
               int cc_ID=cc_twoDarray_ptr->get(px,py);
               int root_ID=uf.Find(cc_ID);
               tmp_twoDarray_ptr->put(px,py,root_ID);

               extremal_region* extremal_region_ptr=NULL;
               iter=coalesced_extremal_region_map_ptr->find(root_ID);
               if (iter==coalesced_extremal_region_map_ptr->end())
               {
                  extremal_region_ptr=new extremal_region(root_ID);
                  (*coalesced_extremal_region_map_ptr)[root_ID]=
                     extremal_region_ptr;
               }
               else
               {
                  extremal_region_ptr=iter->second;
               }   
               extremal_region_ptr->update_bbox(px,py);
            } // loop over px
         } // loop over py
         tmp_twoDarray_ptr->copy(cc_twoDarray_ptr);

         for (iter=coalesced_extremal_region_map_ptr->begin();
              iter != coalesced_extremal_region_map_ptr->end(); iter++)
         {
            extremal_region* extremal_region_ptr=iter->second;
            extremal_region_ptr->run_length_encode(cc_twoDarray_ptr);
         }

//         cout << "coalesced_extremal_region_map_ptr->size() = "
//              << coalesced_extremal_region_map_ptr->size() << endl;
         return coalesced_extremal_region_map_ptr;
      }

      
} // mserfunc namespace
