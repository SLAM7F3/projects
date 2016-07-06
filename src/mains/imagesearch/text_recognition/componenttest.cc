// ==========================================================================
// Program COMPONENTTEST
// ==========================================================================
// Last updated on 7/25/12; 7/29/12; 7/30/12
// ==========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "video/connected_components.h"
#include "image/extremal_region.h"
#include "general/filefuncs.h"
#include "datastructures/Forest.h"
#include "image/graphicsfuncs.h"
#include "general/sysfuncs.h"
#include "image/TwoDarray.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

   int xdim=4;
   int ydim=4;

   connected_components* connected_components_ptr=new connected_components();
//   vector_union_find* union_find_ptr=
//      connected_components_ptr->get_vector_union_find_ptr(); 

   connected_components_ptr->instantiate_binary_image(xdim,ydim);
   connected_components_ptr->initialize_vector_union_find();


   connected_components_ptr->set_binary_image_first();
   connected_components_ptr->recompute_connected_components();
   connected_components_ptr->update_connected_component_properties();
   connected_components_ptr->update_connected_component_labels();
   connected_components_ptr->finalize_connected_component_properties(true);

   connected_components_ptr->print_pbinary_twoDarray();
//   cout << "t=0 *union_find_ptr = " << *union_find_ptr << endl;
   connected_components_ptr->print_prev_cc_twoDarray();
   connected_components_ptr->print_cc_twoDarray();
   connected_components_ptr->copy_cc_onto_prev_cc();


   string banner;
   banner="2222222222222222222222222222222222222222222222222";
   outputfunc::write_big_banner(banner);

   connected_components_ptr->set_binary_image_second();
   connected_components_ptr->recompute_connected_components();
   connected_components_ptr->update_connected_component_properties();
   connected_components_ptr->update_connected_component_labels();
   connected_components_ptr->finalize_connected_component_properties(true);

   connected_components_ptr->print_pbinary_twoDarray();
//   cout << "t=1 *union_find_ptr = " << *union_find_ptr << endl;
   connected_components_ptr->print_prev_cc_twoDarray();
   connected_components_ptr->print_cc_twoDarray();
   connected_components_ptr->copy_cc_onto_prev_cc();

   banner="3333333333333333333333333333333333333333333333333";
   outputfunc::write_big_banner(banner);

   connected_components_ptr->set_binary_image_third();
   connected_components_ptr->recompute_connected_components();
   connected_components_ptr->update_connected_component_properties();
   connected_components_ptr->update_connected_component_labels();
   connected_components_ptr->finalize_connected_component_properties(true);

   connected_components_ptr->print_pbinary_twoDarray();
//   cout << "t=2 *union_find_ptr = " << *union_find_ptr << endl;
   connected_components_ptr->print_prev_cc_twoDarray();
   connected_components_ptr->print_cc_twoDarray();
   connected_components_ptr->copy_cc_onto_prev_cc();

   
   banner="4444444444444444444444444444444444444444444444444";
   outputfunc::write_big_banner(banner);
   

   connected_components_ptr->set_binary_image_fourth();
   connected_components_ptr->recompute_connected_components();
   connected_components_ptr->update_connected_component_properties();
   connected_components_ptr->update_connected_component_labels();
   connected_components_ptr->finalize_connected_component_properties(true);

   connected_components_ptr->print_pbinary_twoDarray();
//   cout << "t=2 *union_find_ptr = " << *union_find_ptr << endl;
   connected_components_ptr->print_prev_cc_twoDarray();
   connected_components_ptr->print_cc_twoDarray();
   connected_components_ptr->copy_cc_onto_prev_cc();



   delete connected_components_ptr;

}
