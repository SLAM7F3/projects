// ==========================================================================
// CONNECTFUNCS stand-alone methods
// ==========================================================================
// Last modified on 12/1/10; 12/4/10; 7/28/12; 4/5/14
// ==========================================================================

#include "math/basic_math.h"
#include "image/binaryimagefuncs.h"
#include "image/connectfuncs.h"
#include "math/constant_vectors.h"
#include "image/extremal_region.h"
#include "graphs/graphfuncs.h"
#include "datastructures/Hashtable.h"
#include "image/imagefuncs.h"
#include "datastructures/Linkedlist.h"
#include "datastructures/Mynode.h"
#include "templates/mytemplates.h"
#include "math/prob_distribution.h"
#include "math/threevector.h"
#include "datastructures/treenode.h"
#include "image/TwoDarray.h"

#include "templates/mytemplates.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

namespace connectfunc
{

// ==========================================================================
// Run length encoding/decoding & connected components methods
// ==========================================================================

// Method run_length_encode takes in a binary thresholded twoDarray.
// It encodes each run of 1-pixels by its row and the columns of its
// starting and ending pixels.  A dummy permutation label holding the
// component label of each run is also initialized to zero.  The RLE
// binary image is returned in a dynamically generated linked list.
// This method follows the RLE conventions spelled out in section
// 2.3.6 of "Computer and Robot Vision" by Haralick and Shapiro (TA
// 1632.H37 vol 1, 1992).  It is needed for binary image connected
// components labeling.  

   linkedlist* run_length_encode(const twoDarray* zbinary_twoDarray_ptr)
      {
         const int n_node_depend_vars=4;
         const double TINY=1E-5;
         double func_value[n_node_depend_vars];
         func_value[3]=0;  // connectivity label initialized to zero

         linkedlist* run_list_ptr=new linkedlist;

         int run_number=0;
         for (unsigned int py=0; py<zbinary_twoDarray_ptr->get_ndim(); py++)
         {
            bool running=false;
            int start_column=0;
            int stop_column=0;
            for (unsigned int px=0; px<zbinary_twoDarray_ptr->get_mdim(); px++)
            {
               if (zbinary_twoDarray_ptr->get(px,py) > TINY)
               {
                  if (!running)
                  {
                     running=true;
                     start_column=px;
                  }
               }
               else
               {
                  if (running)
                  {
                     stop_column=px-1;
                     running=false;
//                     cout << "run_number = " << run_number
//                          << " row = " << py
//                          << " start_column = " << start_column
//                          << " stop_column = " << stop_column 
//                          << endl;
//                     outputfunc::newline();
                     func_value[0]=py;
                     func_value[1]=start_column;
                     func_value[2]=stop_column;
                     run_list_ptr->append_node(
                        datapoint(n_node_depend_vars,run_number,func_value));
                     run_number++;
                  }
               }
            } // loop over px index

// Terminate any runs which reach end of binary image's current row:

            if (running)
            {
               stop_column=zbinary_twoDarray_ptr->get_mdim()-1;
//               cout << "run_number = " << run_number
//                    << " row = " << py
//                    << " start_column = " << start_column
//                    << " stop_column = " << stop_column 
//                    << endl;
//               outputfunc::newline();
               func_value[0]=py;
               func_value[1]=start_column;
               func_value[2]=stop_column;
               run_list_ptr->append_node(
                  datapoint(n_node_depend_vars,run_number,func_value));
               run_number++;
            }
         } // loop over py index 
         return run_list_ptr;
      }

// ---------------------------------------------------------------------
// Method run_length_decode takes in and parses a linked list of
// non-null pixel runs.  It returns within output twoDarray
// *ztwoDarray_ptr a reconstructed image based upon the run length
// information within the input linked list.

   void run_length_decode(
      const linkedlist* run_list_ptr,twoDarray* ztwoDarray_ptr)
      {
         ztwoDarray_ptr->clear_values();
         const mynode* currnode_ptr=run_list_ptr->get_start_ptr();
         while (currnode_ptr != NULL)
         {
            unsigned int py=basic_math::round(
               currnode_ptr->get_data().get_func(0));
            unsigned int px_start=basic_math::round(
               currnode_ptr->get_data().get_func(1));
            unsigned int px_stop=basic_math::round(
               currnode_ptr->get_data().get_func(2));
            double intensity_value=currnode_ptr->get_data().get_func(3);
            for (unsigned int px=px_start; px<=px_stop; px++)
            {
               ztwoDarray_ptr->put(px,py,intensity_value);
            }
            currnode_ptr=currnode_ptr->get_nextptr();
         }
      }

// ---------------------------------------------------------------------
// Method convert_run_to_pixel_list takes in mynode *run_node_ptr
// along with a corresponding image contained within *ztwoDarray_ptr.
// It expands the contents of the run and saves each pixel's px, py, z
// value and connected component information into a linked list.  The
// dynamically generated list is returned by this method.

   linkedlist* convert_run_to_pixel_list(
      const mynode* run_node_ptr,const twoDarray* ztwoDarray_ptr)
      {
         const int n_node_indep_vars=4;
         const int n_node_depend_vars=2;
         double var[n_node_indep_vars];
         double func_value[n_node_depend_vars];

         unsigned int py=basic_math::round(
            run_node_ptr->get_data().get_func(0));
         unsigned int px_start=basic_math::round(
            run_node_ptr->get_data().get_func(1));
         unsigned int px_stop=basic_math::round(
            run_node_ptr->get_data().get_func(2));
         double connected_component_label=run_node_ptr->get_data().
            get_func(3);
         
         if (px_stop < px_start)
         {
            cout << "Error in connectfunc::convert_run_to_pixel_list()"
                 << endl;
            cout << "px_start = " << px_start << " px_stop = " << px_stop
                 << endl;
            cout << "py = " << py 
                 << " label = " << connected_component_label << endl;
         }

         linkedlist* pixel_list_ptr=new linkedlist;
         for (unsigned int px=px_start; px<=px_stop; px++)
         {
            var[0]=px;
            var[1]=py;
            ztwoDarray_ptr->pixel_to_point(px,py,var[2],var[3]);
            func_value[0]=ztwoDarray_ptr->get(px,py);
            func_value[1]=connected_component_label;
            pixel_list_ptr->append_node(
               datapoint(n_node_indep_vars,n_node_depend_vars,
                         var,func_value));
         } // loop over px index
         return pixel_list_ptr;
      }
 
// ---------------------------------------------------------------------
// This overloaded version of run_length_encode returns an array
// RLE_list_ptr[] of linked list pointers.  The array's independent
// variable corresponds to row number.  The address within each array
// cell corresponds to the start of a linked list of runs for each row.

   void run_length_encode(
      const twoDarray* zbinary_twoDarray_ptr,linkedlist* RLE_list_ptr[])
      {

// Allocate RLE_list_ptr[] array:

         for (unsigned int py=0; py<zbinary_twoDarray_ptr->get_ndim(); py++)
         {
            RLE_list_ptr[py]=new linkedlist;
         }

         const int n_node_depend_vars=3;
         const double TINY=1E-5;
         double func_value[n_node_depend_vars];
         func_value[2]=0;	// permutation label

         int run_number=0;
         for (unsigned int py=0; py<zbinary_twoDarray_ptr->get_ndim(); py++)
         {
            bool running=false;
            int start_column=0;
            int stop_column=0;
            for (unsigned int px=0; px<zbinary_twoDarray_ptr->get_mdim(); px++)
            {
               if (zbinary_twoDarray_ptr->get(px,py) > TINY)
               {
                  if (!running)
                  {
                     running=true;
                     start_column=px;
                  }
               }
               else
               {
                  if (running)
                  {
                     stop_column=px-1;
                     running=false;
//                     cout << "run_number = " << run_number
//                          << " row = " << py
//                          << " start_column = " << start_column
//                          << " stop_column = " << stop_column 
//                          << endl;
//                     outputfunc::newline();
                     func_value[0]=start_column;
                     func_value[1]=stop_column;
                     RLE_list_ptr[py]->append_node(
                        datapoint(n_node_depend_vars,run_number,func_value));
                     run_number++;
                  }
               }
            } // loop over px index

// Terminate any runs which reach end of binary image's current row:

            if (running)
            {
               stop_column=zbinary_twoDarray_ptr->get_mdim()-1;
//               cout << "run_number = " << run_number
//                    << " row = " << py
//                    << " start_column = " << start_column
//                    << " stop_column = " << stop_column 
//                    << endl;
//               outputfunc::newline();
               func_value[0]=start_column;
               func_value[1]=stop_column;
               RLE_list_ptr[py]->append_node(
                  datapoint(n_node_depend_vars,run_number,func_value));
               run_number++;
            }
         } // loop over py index 
      }

// ---------------------------------------------------------------------
// Method connect_runs takes in a linked list containing binary image
// information in run length encoded format.  It also takes in the
// binary image itself within twoDarray *zbinary_twoDarray_ptr.  This
// method computes the image's 4-connected or 8-connected components,
// depending upon input boolean parameter eight_connected.  Upon
// completion, this method returns the number nconnected_runs of
// connected components, and it adjust the connectivity labels for
// each node of *run_list_ptr from 0 to nconnected_runs-1.

// The basic connected components algorithm implemented here was taken
// in spirit from MATLAB's BWLABEL.  A description of it is given in
// "Computer and Robot Vision" by Haralick and Shapiro, pp 40 - 48.
// This algorithm also heavily relies upon the find-union algorithm
// and code given in chapter 1 of "Algorithms in C" by Sedgewick.

   int connect_runs(
      linkedlist* run_list_ptr,const twoDarray* zbinary_twoDarray_ptr,
      bool eight_connected)
      {
         unsigned int ndim=zbinary_twoDarray_ptr->get_ndim();
         linkedlist* RLE_list_ptr[ndim];
         connectfunc::run_length_encode(zbinary_twoDarray_ptr,RLE_list_ptr);

         unsigned int nruns=run_list_ptr->size();
         int nconnected_runs=0;
         if (nruns <= 1)
         {
            nconnected_runs=nruns;
         }
         else
         {
            int id[nruns],size[nruns];
            graphfunc::initialize_findunion_structures(nruns,id,size);
            for (unsigned int n=1; n<ndim; n++)
            {
               connect_top_down_runs(
                  nruns,n,id,size,RLE_list_ptr,eight_connected);
            }

            for (unsigned int n=ndim-2; n>=0; n--)
            {
               connect_bottom_up_runs(
                  nruns,n,id,size,RLE_list_ptr,eight_connected);
            }

            nconnected_runs=relabel_runs(nruns,id,run_list_ptr);
         }

// Before exiting this method, delete each linked list within
// dynamically allocated RLE_list_ptr[] array:

         for (unsigned int n=0; n<ndim; n++) delete RLE_list_ptr[n];

         return nconnected_runs;
      }

// ---------------------------------------------------------------------
// Method count_runs takes in the array RLE_list_ptr[] of linked runs
// where the independent array variable corresponds to row number.  It
// returns the number of independent linked lists accessible via this
// data structure.

   void count_runs(unsigned int ndim,linkedlist* RLE_list_ptr[],int& nruns)
      {
         nruns=0;
         for (unsigned int n=0; n<ndim; n++)
         {
            nruns += RLE_list_ptr[n]->size();
         }
      }
   
// ---------------------------------------------------------------------
// Method connect_top_down_runs takes in array RLE_list_ptr[] of
// linked list pointers to pixel runs in a binary image.  It
// sequentially searches through the runs within rows n and n-1.  If a
// run on row n overlaps a row on row n-1, this method places them
// within the same equivalence class via a call to the find-unify
// algoritms.

   void connect_top_down_runs(
      int nruns,int row,int id[],int size[],
      linkedlist* RLE_list_ptr[],bool eight_connected)
      {
         bool four_connected=(!eight_connected);
         mynode* currnode_ptr=RLE_list_ptr[row]->get_start_ptr();

         while (currnode_ptr != NULL)
         {
            int currrun=basic_math::round(currnode_ptr->get_data().get_var(0));
            int currstart=basic_math::round(currnode_ptr->get_data().
                                          get_func(0));
            int currstop=basic_math::round(currnode_ptr->get_data().
                                         get_func(1));
//            cout << "currrun = " << currrun 
//                 << " currstart = " << currstart 
//                 << " currstop = " << currstop << endl;

            mynode* prevrow_node_ptr=RLE_list_ptr[row-1]->get_start_ptr();
            while (prevrow_node_ptr != NULL)
            {
               int prevrun=basic_math::round(
                  prevrow_node_ptr->get_data().get_var(0));
               int prevstart=basic_math::round(
                  prevrow_node_ptr->get_data().get_func(0));
               int prevstop=basic_math::round(
                  prevrow_node_ptr->get_data().get_func(1));
//               cout << "prevrun = " << prevrun 
//                    << " prevstart = " << prevstart 
//                    << " prevstop = " << prevstop << endl;
               
               if (
                  (four_connected && 
                   ((currstart >= prevstart && currstart <= prevstop) ||
                    (currstop >= prevstart && currstop <= prevstop))) ||
                  (eight_connected &&
                     ((currstart>=prevstart-1 && currstart<=prevstop+1) ||
                      (currstop>=prevstart-1 && currstop<=prevstop+1)))
                  )
               {
                  graphfunc::weightedunion(currrun,prevrun,nruns,id,size);
//                  for (unsigned int i=0; i<nruns; i++)
//                  {
//                     cout << id[i] << " " << flush;
//                  }
//                  outputfunc::newline();
               }
               prevrow_node_ptr=prevrow_node_ptr->get_nextptr();
            }
            currnode_ptr=currnode_ptr->get_nextptr();
         } // top-down while loop
      }

// ---------------------------------------------------------------------
// Method connect_bottom_up_runs takes in array RLE_list_ptr[] of
// linked list pointers to pixel runs in a binary image.  It
// sequentially searches through the runs within rows n and n+1.  If a
// run on row n overlaps a row on row n+1, this method places them
// within the same equivalence class via a call to the find-unify
// algoritms.

   void connect_bottom_up_runs(
      int nruns,int row,int id[],int size[],
      linkedlist* RLE_list_ptr[],bool eight_connected)
      {
         bool four_connected=(!eight_connected);
         mynode* currnode_ptr=RLE_list_ptr[row]->get_start_ptr();

         while (currnode_ptr != NULL)
         {
            int currrun=basic_math::round(currnode_ptr->get_data().get_var(0));
            int currstart=basic_math::round(currnode_ptr->get_data().
                                          get_func(0));
            int currstop=basic_math::round(currnode_ptr->get_data().
                                         get_func(1));
//            cout << "currrun = " << currrun 
//                 << " currstart = " << currstart 
//                 << " currstop = " << currstop << endl;

            mynode* nextrow_node_ptr=RLE_list_ptr[row+1]->get_start_ptr();
            while (nextrow_node_ptr != NULL)
            {
               int nextrun=basic_math::round(
                  nextrow_node_ptr->get_data().get_var(0));
               int nextstart=basic_math::round(
                  nextrow_node_ptr->get_data().get_func(0));
               int nextstop=basic_math::round(
                  nextrow_node_ptr->get_data().get_func(1));
//               cout << "nextrun = " << nextrun 
//                    << " nextstart = " << nextstart 
//                    << " nextstop = " << nextstop << endl;
               
               if (
                  (four_connected && 
                   ((currstart >= nextstart && currstart <= nextstop) ||
                    (currstop >= nextstart && currstop <= nextstop))) ||
                  (eight_connected &&
                     ((currstart>=nextstart-1 && currstart<=nextstop+1) ||
                      (currstop>=nextstart-1 && currstop<=nextstop+1)))
                  )
               {
                  graphfunc::weightedunion(currrun,nextrun,nruns,id,size);
//                  for (unsigned int i=0; i<nruns; i++)
//                  {
//                     cout << id[i] << " " << flush;
//                  }
//                  outputfunc::newline();
               }
               nextrow_node_ptr=nextrow_node_ptr->get_nextptr();
            }
            currnode_ptr=currnode_ptr->get_nextptr();
         } // bottom-up while loop
      }

// ---------------------------------------------------------------------
// Method relabel_runs takes in integer array id[] generated by method
// connect_runs().  It returns the number n_connected_runs of runs
// which are either 4-connected or 8-connected.  It also relabels the
// entry within the id array so that they uniformly increase from 0 to
// n_connected_runs-1.  This method assumes at that at least 1
// nontrivial run exists.

   int relabel_runs(unsigned int nruns,int id[],linkedlist* run_list_ptr)
      {
         int root[nruns],sorted_root[nruns],orig_posn[nruns];
         for (unsigned int n=0; n<nruns; n++)
         {
            root[n]=graphfunc::tree_root(n,nruns,id);
         }

         for (unsigned int n=0; n<nruns; n++)
         {
            sorted_root[n]=root[n];
            orig_posn[n]=n;
         }
         Quicksort(sorted_root,orig_posn,nruns);

         int n_connected_runs=1;
         int new_root[nruns];
         new_root[0]=0;
         for (unsigned int n=1; n<nruns; n++)
         {
            if (sorted_root[n] > sorted_root[n-1])
            { 
               n_connected_runs++;
               new_root[n]=new_root[n-1]+1;
            }
            else
            {
               new_root[n]=new_root[n-1];
            }
         }

//         outputfunc::newline();
//         cout << "inside connectfunc::relabel_runs()" << endl;
//         cout << "n_connected_runs = " << n_connected_runs << endl;
//         for (unsigned int n=0; n<nruns; n++)
//         {
//            cout << "n=" << n << " id=" << id[n]
//                 << " root=" << root[n]
//                 << " sorted_root=" << sorted_root[n]
//                 << " orig_posn=" << orig_posn[n]
//                 << " new_root=" << new_root[n] << endl;
//         }
         
         for (unsigned int m=0; m<nruns; m++)
         {
            int n=orig_posn[m];
            id[n]=new_root[m];
         }

         int n=0;
         mynode* currnode_ptr=run_list_ptr->get_start_ptr();
         while (currnode_ptr != NULL)
         {
            currnode_ptr->get_data().set_func(3,id[n++]); 
				// connected component label
            currnode_ptr=currnode_ptr->get_nextptr();
         }
//         cout << "run_list = " << *run_list_ptr << endl;
         return n_connected_runs;
      }

// ---------------------------------------------------------------------
// Method generate_connected_hashtable takes in a linked list of pixel
// runs.  The connected components to which these runs belong are
// assumed to have already been computed prior to this method being
// called.  Each connected component is stored within a hash table
// that is dynamically generated by this method.  The elements in the
// hash table are addresses to linked lists of pixels which make up
// each connected component.

   Hashtable<linkedlist*>* generate_connected_hashtable(
      int n_connected_runs,unsigned int min_component_pixels,
      linkedlist* run_list_ptr,twoDarray const *ztwoDarray_ptr)
      {

// Set size of hashtable to 2 times the current number of connected
// runs in order to minimize collisions:

         Hashtable<linkedlist*>* connected_hashtable_ptr=
            new Hashtable<linkedlist*>(basic_math::round(2*n_connected_runs));

// Traverse runs linked list and generate pixel list for each run:

         for (mynode* run_node_ptr=run_list_ptr->get_start_ptr();
              run_node_ptr != NULL; run_node_ptr=run_node_ptr->get_nextptr())
         {
            linkedlist* pixel_list_ptr=convert_run_to_pixel_list(
               run_node_ptr,ztwoDarray_ptr);
               
// Set key for each connected component equal to its label:

            int key=basic_math::round(run_node_ptr->get_data().get_func(3)); 
            Mynode<linkedlist*>* keynode_ptr=
               connected_hashtable_ptr->retrieve_key(key);
            if (keynode_ptr==NULL)
            {
               keynode_ptr=connected_hashtable_ptr->insert_key(
                  key,pixel_list_ptr);
            }
            else
            {
               linkedlist* existing_list_ptr=keynode_ptr->get_data();
               existing_list_ptr->concatenate(pixel_list_ptr);

// On 8/24/04, we discovered (the hard and painful way!) that the
// following delete call leads to segmentation faults within program
// ortho.  We don't understand why, but for now we comment out the
// next line...

//               delete pixel_list_ptr;
            }
         } // loop over nodes in *run_list_ptr

// Delete lists within connected hashtable which contain too few
// connected components:

         int nsmall_components=0;
         for (unsigned int i=0; 
              i<connected_hashtable_ptr->get_table_capacity(); i++)
         {
            Linkedlist<linkedlist*>* currlist_ptr=
               connected_hashtable_ptr->get_list_ptr(i);
            if (currlist_ptr != NULL)
            {
               Mynode<linkedlist*>* currnode_ptr=
                  currlist_ptr->get_start_ptr();
               while (currnode_ptr != NULL)
               {
                  Mynode<linkedlist*>* nextnode_ptr=
                     currnode_ptr->get_nextptr();
                  linkedlist* pixellist_ptr=currnode_ptr->get_data();
                  if (pixellist_ptr->size() < min_component_pixels)
                  {
//                     cout << "min_component_pixels = " << min_component_pixels
//                          << " npixels = " << pixellist_ptr->size()
//                          << endl;
                     nsmall_components++;
                     currlist_ptr->delete_node(currnode_ptr);
                     connected_hashtable_ptr->decrement_nkeys_in_table();
                  }
                  currnode_ptr=nextnode_ptr;
               }
            } // currlist_ptr != NULL conditional
         } // loop over index i labeling connected hashtable's linked lists

         cout << "Number of nulled small connected components = "
              << nsmall_components << endl;
         cout << "Number of surviving sizable connected components = "
              << connected_hashtable_ptr->size() << endl;
         return connected_hashtable_ptr;
      }

// ---------------------------------------------------------------------
// This overloaded version of generate_connected_hashtable operates at
// a higher level (and is more user friendly) than its predecessor.
// It takes in threshold parameter zmin and uses it to generate a
// binary version of the input twoDarray contained within
// *ztwoDarray_ptr.  It next run-length-encodes the binary image.
// Finally, this method dynamically generates and returns a hashtable
// containing each of the connected components.

   Hashtable<linkedlist*>* generate_connected_hashtable(
      double zthreshold,int min_component_pixels,
      twoDarray const *ztwoDarray_ptr,bool threshold_below_cutoff)
      {
         twoDarray* zbinary_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
         if (threshold_below_cutoff)
         {
            binaryimagefunc::binary_threshold(
               zthreshold,ztwoDarray_ptr,zbinary_twoDarray_ptr);
         }
         else
         {
            binaryimagefunc::binary_threshold_above_cutoff(
               zthreshold,ztwoDarray_ptr,zbinary_twoDarray_ptr);
         }
         linkedlist* run_list_ptr=run_length_encode(zbinary_twoDarray_ptr);
         int n_connected_runs=connectfunc::connect_runs(
            run_list_ptr,zbinary_twoDarray_ptr);
         delete zbinary_twoDarray_ptr;

         cout << "n_connected_runs = " << n_connected_runs << endl;
         
         Hashtable<linkedlist*>* connected_hashtable_ptr=
            generate_connected_hashtable(
               n_connected_runs,min_component_pixels,run_list_ptr,
               ztwoDarray_ptr);
         delete run_list_ptr;
         return connected_hashtable_ptr;
      }

// ---------------------------------------------------------------------
// Method decode_connected_hashtable traverses through the connected
// components hashtable *connected_hashtable_ptr and extracts each
// stored linked pixel list.  It sets the intensities of each pixel
// within a single connected component equal to a unique value, and it
// writes the connected component results to output twoDarray
// *ptwoDarray_ptr.

   void decode_connected_hashtable(
      Hashtable<linkedlist*>* connected_hashtable_ptr,
      twoDarray* ptwoDarray_ptr,
      bool set_component_intensities_less_than_unity,double null_value)
      {
//         cout << "inside connectfunc::decode_connected_hashtable()" << endl;
//         cout << "null_value = " << null_value << endl;

         ptwoDarray_ptr->initialize_values(null_value);
         const int ncolors=11;
         double intensity[ncolors]={10,25,40,55,15,30,45,60,20,35,50};
         if (set_component_intensities_less_than_unity)
         {
            intensity[0]=0.1;
            intensity[1]=0.4;
            intensity[2]=0.7;
            intensity[3]=0.2;
            intensity[4]=0.5;
            intensity[5]=0.8;
            intensity[6]=0.3;
            intensity[7]=0.6;
            intensity[8]=0.9;
            intensity[9]=0.25;
            intensity[10]=0.65;
         }

         int nlist=0;
         for (unsigned int j=0; 
              j<connected_hashtable_ptr->get_table_capacity(); j++)
         {
            Linkedlist<linkedlist*>* hashlist_ptr=
               connected_hashtable_ptr->get_list_ptr(j);
            if (hashlist_ptr != NULL)
            {
               for (Mynode<linkedlist*>* hashnode_ptr=hashlist_ptr->
                       get_start_ptr(); hashnode_ptr != NULL;
                    hashnode_ptr=hashnode_ptr->get_nextptr())
               {
                  linkedlist* currlist_ptr=hashnode_ptr->get_data();

                  if (currlist_ptr != NULL)
                  {
                     double curr_intensity=intensity[modulo(nlist,ncolors)];
//                     cout << "curr_intensity = " << curr_intensity << endl;
                     for (mynode* curr_pixel_ptr=currlist_ptr->
                             get_start_ptr();
                          curr_pixel_ptr != NULL; curr_pixel_ptr=
                             curr_pixel_ptr->get_nextptr())
                     {
                        int px=basic_math::round(
                           curr_pixel_ptr->get_data().get_var(0));
                        int py=basic_math::round(
                           curr_pixel_ptr->get_data().get_var(1));
                        ptwoDarray_ptr->put(px,py,curr_intensity);
                     }
                     nlist++;
                  } // currlist_ptr != NULL conditional
               } // loop over nodes in *hashlist_ptr
            } // hashlist_ptr != NULL conditional
         } // loop over index j labeling hashtable's linked lists
      }

// ---------------------------------------------------------------------
// Method delete_connected_hashtable destroys each of the dynamically
// allocated pixel linked lists which correspond to the entries within
// the connected components hashtable.  It subsequently deallocates the
// hashtable itself.

   void delete_connected_hashtable(
      Hashtable<linkedlist*>* connected_hashtable_ptr)
      {
         if (connected_hashtable_ptr != NULL)
         {
            for (unsigned int i=0; i<connected_hashtable_ptr->get_table_capacity(); 
                 i++)
            {
               Linkedlist<linkedlist*>* currlist_ptr=
                  connected_hashtable_ptr->get_list_ptr(i);
                      if (currlist_ptr != NULL)
               {
                  Mynode<linkedlist*>* currnode_ptr=
                     currlist_ptr->get_start_ptr();
                  while (currnode_ptr != NULL)
                  {
                     Mynode<linkedlist*>* nextnode_ptr=
                        currnode_ptr->get_nextptr();
                     linkedlist* pixellist_ptr=currnode_ptr->get_data();
                     delete pixellist_ptr;
                     currnode_ptr=nextnode_ptr;
                  }
               } // currlist_ptr != NULl conditional
            } // loop over index i labeling connected hashtable's linked lists
            delete connected_hashtable_ptr;
         }
      }

// ==========================================================================
// Largest connected component methods
// ==========================================================================

// Method largest_connected_component() loops over all connected
// components stored within input *connected_hashtable_ptr.  It sorts
// these components by their numbers of pixels.  This method returns
// within the pixellist corresponding to the largest connected
// component.

   linkedlist* largest_connected_component(
      Hashtable<linkedlist*>* connected_hashtable_ptr)
      {
         vector<int> pixellist_size;
         vector<linkedlist*> pixellist_ptrs;
         
         if (connected_hashtable_ptr != NULL)
         {
            for (unsigned int i=0; i<connected_hashtable_ptr->get_table_capacity(); 
                 i++)
            {
               Linkedlist<linkedlist*>* currlist_ptr=
                  connected_hashtable_ptr->get_list_ptr(i);
                      if (currlist_ptr != NULL)
               {
                  Mynode<linkedlist*>* currnode_ptr=
                     currlist_ptr->get_start_ptr();
                  while (currnode_ptr != NULL)
                  {
                     Mynode<linkedlist*>* nextnode_ptr=
                        currnode_ptr->get_nextptr();
                     linkedlist* pixellist_ptr=currnode_ptr->get_data();

                     pixellist_size.push_back(pixellist_ptr->size());
                     pixellist_ptrs.push_back(pixellist_ptr);

                     currnode_ptr=nextnode_ptr;
                  }
               } // currlist_ptr != NULl conditional
            } // loop over index i labeling connected hashtable's linked lists
         }

         templatefunc::Quicksort_descending(pixellist_size,pixellist_ptrs);
         for (unsigned int i=0; i<pixellist_size.size(); i++)
         {
//            cout << "i = " << i
//                 << " pixellist_size = " << pixellist_size[i]
//                 << endl;
         }

         return pixellist_ptrs[0];
      }

// ---------------------------------------------------------------------
// Method pixel_connected_to_component() takes in a candidate pixel
// location (px,py) along with a binary image in
// *zconnected_twoDarray_ptr which is assumed to consist of a single
// connected component.  If the input pixel is 8-connected to any part
// *zconnected_twoDarray_ptr, this boolean method returns true.

   bool pixel_connected_to_component(
      unsigned int px,unsigned int py,twoDarray* zconnected_twoDarray_ptr)
      {
         if (nearly_equal(zconnected_twoDarray_ptr->get(px,py),1))
         {
            return true;
         }

         const int offset=3;
         
         unsigned int px_min=basic_math::max(Unsigned_Zero,px-offset);
         unsigned int px_max=basic_math::min(
            zconnected_twoDarray_ptr->get_mdim()-1,px+offset);
         unsigned int py_min=basic_math::max(Unsigned_Zero,py-offset);
         unsigned int py_max=basic_math::min(
            zconnected_twoDarray_ptr->get_ndim()-1,py+offset);
         
//         cout << "px = " << px << " px_min = " << px_min
//              << " px_max = " << px_max << endl;
//         cout << "py = " << py << " py_min = " << py_min
//              << " py_max = " << py_max << endl;

         for (unsigned int qx=px_min; qx <= px_max; qx++)
         {
            for (unsigned int qy=py_min; qy <= py_max; qy++)
            {
//               cout << "zconnected = " << zconnected_twoDarray_ptr->get(qx,qy)
//                    << endl;
               if (nearly_equal(zconnected_twoDarray_ptr->get(qx,qy),1))
                  return true;
            }
         }

         return false;
      }
   
// ---------------------------------------------------------------------
// Method aggregate_pixels_into_connected_component()

   int aggregate_pixels_into_connected_component(
      const twoDarray* zorig_twoDarray_ptr,twoDarray* zconnected_twoDarray_ptr)
      {
         int n_pixels_changed=0;
         for (unsigned int px=0; px<zorig_twoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<zorig_twoDarray_ptr->get_ndim(); py++)
            {
               int curr_z=zorig_twoDarray_ptr->get(px,py);
               if (curr_z != 1) continue;

               int connected_z=zconnected_twoDarray_ptr->get(px,py);
               if (connected_z==1) continue;
               
               if (pixel_connected_to_component(
                  px,py,zconnected_twoDarray_ptr))
               {
                  zconnected_twoDarray_ptr->put(px,py,1);
                  n_pixels_changed++;
               }
            } // loop over py
         } // loop over px

         return n_pixels_changed;
      }

// ---------------------------------------------------------------------
// Method fill_connected_component_from_original_image

   void fill_connected_component_from_original_image(
      const twoDarray* zorig_twoDarray_ptr,twoDarray* zconnected_twoDarray_ptr)
      {
         int iter=0;
         int n_pixels_changed=1;
         while(n_pixels_changed > 0)
         {
            n_pixels_changed=aggregate_pixels_into_connected_component(
               zorig_twoDarray_ptr,zconnected_twoDarray_ptr);
            cout << "iter = " << iter 
                 << " n_pixels_changed = " << n_pixels_changed << endl;
         }
      }

// ==========================================================================
// Quasi derivative methods
// ==========================================================================

// Method compute_local_pixel_intensity_variation takes in linked list
// of pixels belong to some connected component.  It also takes in
// twoDarrays *ztwoDarray_ptr and *zbinary_twoDarray_ptr containing
// the image and binary image containing the connected component.  For
// each pixel within the linked list, this method computes the sum of
// the absolute differences between the pixel's intensity value and
// those of its nearest non-null neighbors (as defined by the binary
// image).  The average of all these absolute differences is returned
// within output parameter avg_global_delta_f.  
      
   bool compute_local_pixel_intensity_variation(
      const linkedlist* pixel_list_ptr,
      const twoDarray* ztwoDarray_ptr,const twoDarray* zbinary_twoDarray_ptr,
      double& avg_global_delta_f)
      {
         bool intensity_variation_computed=true;
         if (pixel_list_ptr != NULL)
         {
            double global_delta_f=0;
            const mynode* curr_pixel_ptr=pixel_list_ptr->get_start_ptr();
            while (curr_pixel_ptr != NULL)
            {
               int px=basic_math::round(curr_pixel_ptr->get_data().get_var(0));
               int py=basic_math::round(curr_pixel_ptr->get_data().get_var(1));
               double curr_intensity=curr_pixel_ptr->get_data().get_func(0);

               int n_neighbors=0;
               double delta_f=0;
               for (int i=-1; i<=1; i++)
               {
                  for (int j=-1; j<=1; j++)
                  {
                     if (ztwoDarray_ptr->pixel_inside_working_region(
                        px+i,py+j) 
                         && zbinary_twoDarray_ptr->get(px+i,py+j) > 0.5)
                     {
                        delta_f += 
                           fabs(curr_intensity-ztwoDarray_ptr->get(
                              px+i,py+j));
                        n_neighbors++;
                     }
                  } // loop over index j
               } // loop over index i

               double local_avg_delta_f=0;
               if (n_neighbors > 0)
               {
                  local_avg_delta_f=delta_f/double(n_neighbors);
               }
               global_delta_f += local_avg_delta_f;
               curr_pixel_ptr=curr_pixel_ptr->get_nextptr();
            } // while loop
            avg_global_delta_f=
               global_delta_f/double(pixel_list_ptr->size());
         }
         else
         {
            intensity_variation_computed=false;
            avg_global_delta_f=POSITIVEINFINITY;
         }
         return intensity_variation_computed;
      }

// ==========================================================================
// Pixel list properties methods
// ==========================================================================

// Method pixel_list_COM takes in a linked pixel list corresponding to
// some connected component within the input twoDarray
// *ztwoDarray_ptr.  It computes the center-of-mass of the connected
// component.  If boolean input parameter compute_binary_COM==true,
// only shape (as opposed to intensity) information is used to
// determine the center-of-mass.

   threevector pixel_list_COM(
      const linkedlist* pixel_list_ptr,twoDarray const *ztwoDarray_ptr,
      bool compute_binary_COM,bool use_pixel_coords)
      {
         threevector COM(Zero_vector);
         bool COM_calculated_successfully=false;
         if (pixel_list_ptr != NULL)
         {
            double currx,curry,currz,xsum,ysum,zsum;
            xsum=ysum=zsum=0;
            for (const mynode* curr_pixel_ptr=pixel_list_ptr->get_start_ptr();
                 curr_pixel_ptr != NULL; curr_pixel_ptr=curr_pixel_ptr->
                    get_nextptr())
            {
               if (use_pixel_coords)
               {
                  int px=basic_math::round(curr_pixel_ptr->get_data().
                                         get_var(0));
                  int py=basic_math::round(curr_pixel_ptr->get_data().
                                         get_var(1));
                  ztwoDarray_ptr->pixel_to_point(px,py,currx,curry);
               }
               else
               {
                  currx=curr_pixel_ptr->get_data().get_var(2);
                  curry=curr_pixel_ptr->get_data().get_var(3);
               }

               if (compute_binary_COM) 
               {
                  currz=1;
               }
               else
               {
                  currz=curr_pixel_ptr->get_data().get_func(0);
               }
               xsum += currz*currx;
               ysum += currz*curry;
               zsum += currz;
            }

            if (zsum > 0)
            {
               COM.put(0,xsum/zsum);
               COM.put(1,ysum/zsum);
               COM.put(2,0);
               COM_calculated_successfully=true;
            }
         } // pixel_list_ptr != NULL conditional
      
         if (!COM_calculated_successfully)
         {
            cout << "Problem in connectfunc::pixel_list_COM()" << endl;
            cout << "COM not successfully calculated !" << endl;
         }
         return COM;
      }

// ---------------------------------------------------------------------
// Method pixel_list_moi takes in a linked pixel list corresponding to
// some connected component within the input twoDarray
// *ztwoDarray_ptr.  It computes the moment of inertia eignevalues
// with respect to the input origin point for the connected component.
// If boolean input parameter compute_binary_COM==true, only shape (as
// opposed to intensity) information is used to determine the
// center-of-mass.

// Note: This method has been essentially copied from
// imagefunc::moment_of_inertia().  Someday, we'll clean up this
// method to avoid needless code duplication!

   bool pixel_list_moi(
      const linkedlist* pixel_list_ptr,twoDarray const *ztwoDarray_ptr,
      const threevector& origin,double& Imin,double& Imax,
      threevector& Imin_hat,threevector& Imax_hat,bool compute_binary_moi)
      {
         bool moi_calculated_successfully=false;
         if (pixel_list_ptr != NULL)
         {
            double currx,curry,currz,xsqsum,ysqsum,xysum,zsum;
            xsqsum=ysqsum=xysum=zsum=0;
            const mynode* curr_pixel_ptr=pixel_list_ptr->get_start_ptr();
            while (curr_pixel_ptr != NULL)
            {
               int px=basic_math::round(
                  curr_pixel_ptr->get_data().get_var(0));
               int py=basic_math::round(
                  curr_pixel_ptr->get_data().get_var(1));
               ztwoDarray_ptr->pixel_to_point(px,py,currx,curry);
               double dx=currx-origin.get(0);
               double dy=curry-origin.get(1);

               if (compute_binary_moi) 
               {
                  currz=1;
               }
               else
               {
                  currz=ztwoDarray_ptr->get(px,py);
               }
               xsqsum += currz*sqr(dx);
               ysqsum += currz*sqr(dy);
               xysum += currz*(dx*dy);
               zsum += currz;
               curr_pixel_ptr=curr_pixel_ptr->get_nextptr();
            }
            if (zsum > 0)
            {
               double Ixx=ysqsum/zsum;
               double Iyy=xsqsum/zsum;
               double Ixy=-xysum/zsum;
               double I1=(Ixx+Iyy+sqrt(sqr(Ixx-Iyy)+4*sqr(Ixy)))/2;
               double I2=(Ixx+Iyy-sqrt(sqr(Ixx-Iyy)+4*sqr(Ixy)))/2;
               Imin=basic_math::min(I1,I2);
               Imax=basic_math::max(I1,I2);
               moi_calculated_successfully=true;

// If Ixy vanishes, then the principle axes lie in the x and y directions:

               const double TINY=1E-8;
//               double theta_max,theta_min2,theta_max2;
               double theta_min;
               if (fabs(Ixy) < TINY)
               {
                  if (Ixx < Iyy)
                  {
                     theta_min=0;
//                     theta_max=PI/2;
                  }
                  else
                  {
                     theta_min=PI/2;
//                     theta_max=0;
                  }
               }
               else
               {
                  theta_min=-atan2(Ixy,Iyy-Imin);
//                  theta_max=-atan2(Ixy,Iyy-Imax);
//                  theta_min2=-atan2(Ixx-Imin,Ixy);
//                  theta_max2=-atan2(Ixx-Imax,Ixy);
               }
               Imin_hat=threevector(cos(theta_min),sin(theta_min));
               Imax_hat=threevector(-Imin_hat.get(1),Imin_hat.get(0));
            }
         } // pixel_list_ptr != NULL conditional
         return moi_calculated_successfully;
      }

// ---------------------------------------------------------------------
// Method pixels_close_to_point takes in a linked list of pixels
// corresponding to some connected component within input height image
// *ztwoDarray_ptr.  It also takes in a reference point along with a
// radius parameter.  This method returns a dynamically generated
// linked list containing those pixels within the original linked list
// whose distances from the reference point are less than the radius
// value.

   linkedlist* pixels_close_to_point(
      const linkedlist* pixel_list_ptr,twoDarray const *ztwoDarray_ptr,
      threevector& ref_point,double radius)
      {
         const int n_node_indep_vars=2;
         const int n_node_depend_vars=2;
         double var[n_node_indep_vars];
         double func_value[n_node_depend_vars];
         linkedlist* close_pixels_list_ptr=new linkedlist;

         double radius_sqr=sqr(radius);
         if (pixel_list_ptr != NULL)
         {
            const mynode* curr_pixel_ptr=pixel_list_ptr->get_start_ptr();
            double connected_component_label=
               curr_pixel_ptr->get_data().get_func(1);

            double currx,curry;
            while (curr_pixel_ptr != NULL)
            {
               int px=basic_math::round(
                  curr_pixel_ptr->get_data().get_var(0));
               int py=basic_math::round(
                  curr_pixel_ptr->get_data().get_var(1));
               ztwoDarray_ptr->pixel_to_point(px,py,currx,curry);
               double curr_sqrd_dist=sqr(ref_point.get(0)-currx)+
                  sqr(ref_point.get(1)-curry);
               if (curr_sqrd_dist < radius_sqr)
               {
                  var[0]=px;
                  var[1]=py;
                  func_value[0]=ztwoDarray_ptr->get(px,py);
                  func_value[1]=connected_component_label;
                  close_pixels_list_ptr->append_node(
                     datapoint(n_node_indep_vars,n_node_depend_vars,
                               var,func_value));
               }
               curr_pixel_ptr=curr_pixel_ptr->get_nextptr();
            } // curr_pixels_ptr != NULL while loop
         } // pixel_list_ptr != NULL conditional
         return close_pixels_list_ptr;
      }

// ---------------------------------------------------------------------
// Method similar_height_pixels takes in a linked list of pixels
// corresponding to some connected component within input height image
// *ztwoDarray_ptr.  It also takes in a reference height.  This method
// appends onto linked list *close_pixels_list_ptr those pixels within
// the original linked list whose height differences from the
// reference height are less than the input height tolerance.

   void similar_height_pixels(
      const linkedlist* pixel_list_ptr,twoDarray const *ztwoDarray_ptr,
      const double ref_height,double height_tolerance,
      linkedlist* close_pixels_list_ptr)
      {
         const int n_node_indep_vars=2;
         const int n_node_depend_vars=2;
         double var[n_node_indep_vars];
         double func_value[n_node_depend_vars];

         if (pixel_list_ptr != NULL)
         {
            const mynode* curr_pixel_ptr=pixel_list_ptr->get_start_ptr();
            double connected_component_label=
               curr_pixel_ptr->get_data().get_func(1);

            while (curr_pixel_ptr != NULL)
            {
               int px=basic_math::round(curr_pixel_ptr->get_data().get_var(0));
               int py=basic_math::round(curr_pixel_ptr->get_data().get_var(1));
               double curr_z=ztwoDarray_ptr->get(px,py);
               double delta_height=fabs(ref_height-curr_z);
               if (delta_height < height_tolerance)
               {
                  var[0]=px;
                  var[1]=py;
                  func_value[0]=curr_z;
                  func_value[1]=connected_component_label;
                  close_pixels_list_ptr->append_node(
                     datapoint(n_node_indep_vars,n_node_depend_vars,
                               var,func_value));
               }
               curr_pixel_ptr=curr_pixel_ptr->get_nextptr();
            } // curr_pixels_ptr != NULL while loop
         } // pixel_list_ptr != NULL conditional
      }

// ---------------------------------------------------------------------
// Method pixel_list_height_percentile takes in a linked list of pixel
// coordinate pairs along with a corresponding height image within
// *ztwoDarray_ptr.  It computes the distribution of heights for the
// set of pixels and returns the height corresponding to cumulative
// fraction specified by input parameter height_frac.

   double pixel_list_height_percentile(
      double height_frac,Linkedlist<pair<int,int> > const *pixel_list_ptr,
      twoDarray const *ztwoDarray_ptr)
      {
         int n_pixels=pixel_list_ptr->size();
         int counter=0;
         double z[n_pixels];
         for (const Mynode<pair<int,int> >* currnode_ptr=pixel_list_ptr->
                 get_start_ptr(); currnode_ptr != NULL;
              currnode_ptr=currnode_ptr->get_nextptr())
         {
            int px=currnode_ptr->get_data().first;
            int py=currnode_ptr->get_data().second;
            z[counter++]=ztwoDarray_ptr->get(px,py);
         }
         prob_distribution prob_z(counter,z,30);
         return prob_z.find_x_corresponding_to_pcum(height_frac);
      }
   
// ==========================================================================
// Pixel list display methods:
// ==========================================================================

// Method convert_pixel_list_to_twoDarray takes in linked list
// *pixel_list_ptr which is assumed to contain pixel coordinate or
// pixel position information (depending upon the value of boolean
// parameter pixel_coordinates_flag).  It fills the corresponding
// locations within output twoDarray *ftwoDarray_ptr with the
// connected component label information stored within the input
// linked list.

   void convert_pixel_list_to_twoDarray(
      const linkedlist* pixel_list_ptr,twoDarray* ftwoDarray_ptr)
      {
         if (pixel_list_ptr != NULL)
         {
            const mynode* curr_pixel_ptr=pixel_list_ptr->get_start_ptr();
            while (curr_pixel_ptr != NULL)
            {
               double connected_component_label=
                  curr_pixel_ptr->get_data().get_func(1);
               int px=basic_math::round(curr_pixel_ptr->get_data().get_var(0));
               int py=basic_math::round(curr_pixel_ptr->get_data().get_var(1));
               ftwoDarray_ptr->put(px,py,connected_component_label);
               curr_pixel_ptr=curr_pixel_ptr->get_nextptr();
            } // curr_pixel_ptr != NULL while looop
         } // pixel_list_ptr != NULL conditional
      }

// ---------------------------------------------------------------------
// Method convert_twoDarray_to_pixel_list represents a "quasi-inverse"
// to method convert_pixel_list_to_twoDarray.  It scans over input
// twoDarray *fmask_twoDarray_ptr for pixels whose value equals input
// parameter mask_value.  This method appends the pixel coordinates
// for such pixels (which are assumed to be clustered together within
// a single clump) onto a linked list.  The dynamically generated
// pixel list is returned by this method.

   linkedlist* convert_twoDarray_to_pixel_list(
      double mask_value,twoDarray const *fmask_twoDarray_ptr)
      {
         const int n_node_indep_vars=2;
         const int n_node_depend_vars=2;
         double var[n_node_indep_vars];
         double func_value[n_node_depend_vars];
         func_value[0]=func_value[1]=mask_value;

         linkedlist* pixel_list_ptr=new linkedlist;
         for (unsigned int px=0; px<fmask_twoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<fmask_twoDarray_ptr->get_ndim(); py++)
            {
               if (nearly_equal(fmask_twoDarray_ptr->get(px,py),mask_value))
               {
                  var[0]=px;
                  var[1]=py;
                  pixel_list_ptr->append_node(
                     datapoint(n_node_indep_vars,n_node_depend_vars,
                               var,func_value));
               }
            } // loop over py index
         } // loop over px index
         return pixel_list_ptr;
      }

// ---------------------------------------------------------------------
// Method convert_pixel_list_to_image takes in a linked pixel list and
// returns its image form within output twoDarray *ztwoDarray_ptr.
// The overloaded method below also returns a binary thresholded
// version of the image in *zbinary_twoDarray_ptr.

   bool convert_pixel_list_to_binary_image(
      const linkedlist* pixel_list_ptr,twoDarray* zbinary_twoDarray_ptr)
      {
         bool image_successfully_computed=true;
         if (pixel_list_ptr != NULL)
         {
            zbinary_twoDarray_ptr->clear_values();
            const mynode* curr_pixel_ptr=pixel_list_ptr->get_start_ptr();
            while (curr_pixel_ptr != NULL)
            {
               int px=basic_math::round(curr_pixel_ptr->get_data().get_var(0));
               int py=basic_math::round(curr_pixel_ptr->get_data().get_var(1));
               zbinary_twoDarray_ptr->put(px,py,1);
               curr_pixel_ptr=curr_pixel_ptr->get_nextptr();
            }
         }
         else
         {
            image_successfully_computed=false;
         }
         return image_successfully_computed;
      }

   bool convert_pixel_list_to_image(
      const linkedlist* pixel_list_ptr,twoDarray* ztwoDarray_ptr)
      {
         twoDarray* zbinary_twoDarray_ptr=new twoDarray(
            ztwoDarray_ptr);
         bool image_successfully_computed=
            convert_pixel_list_to_image(
               pixel_list_ptr,ztwoDarray_ptr,zbinary_twoDarray_ptr);
         delete zbinary_twoDarray_ptr;
         return image_successfully_computed;
      }

   bool convert_pixel_list_to_image(
      const linkedlist* pixel_list_ptr,
      twoDarray* ztwoDarray_ptr,twoDarray* zbinary_twoDarray_ptr)
      {
         bool image_successfully_computed=true;
         if (pixel_list_ptr != NULL)
         {
            ztwoDarray_ptr->clear_values();
            zbinary_twoDarray_ptr->clear_values();
            const mynode* curr_pixel_ptr=pixel_list_ptr->get_start_ptr();
            while (curr_pixel_ptr != NULL)
            {
               int px=basic_math::round(
                  curr_pixel_ptr->get_data().get_var(0));
               int py=basic_math::round(
                  curr_pixel_ptr->get_data().get_var(1));
               double curr_intensity=curr_pixel_ptr->get_data().get_func(0);
               ztwoDarray_ptr->put(px,py,curr_intensity);
               zbinary_twoDarray_ptr->put(px,py,1);
               curr_pixel_ptr=curr_pixel_ptr->get_nextptr();
            }
         }
         else
         {
            image_successfully_computed=false;
         }
         return image_successfully_computed;
      }

// ==========================================================================
// Extremal Region pooled memory methods
// ==========================================================================

// Method create_extremal_region_pooled_memory() generates large pools
// for treenodes and extremal regions to minimize dynamic creation
// times for these objects.  As of 7/5/12, we believe a typical image
// can easily have 100K extremal regions across 255 thresholds.

   void create_extremal_region_pooled_memory()
   {
      treenode<extremal_region*>* treenode_ptr;
      treenode_ptr->create_pooled_memory(500000);
      extremal_region er;
      er.create_pooled_memory(500000);
   }

// Method delete_extremal_region_pooled_memory() deletes large memory
// pools for treenodes and extremal regions.
   
   void delete_extremal_region_pooled_memory()
   {
      treenode<extremal_region*>* treenode_ptr;
      treenode_ptr->delete_pooled_memory();
      extremal_region er;
      er.delete_pooled_memory();
   }

} // connectfunc namespace




