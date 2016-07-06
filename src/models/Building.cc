// ==========================================================================
// Building class member function definitions
// ==========================================================================
// Last modified on 4/15/12; 4/16/12; 6/28/12; 4/5/14
// ==========================================================================

#include <string>
#include "models/Building.h"
#include "general/filefuncs.h"
#include "geometry/polyhedron.h"

using std::cout;
using std::endl;
using std::map;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void Building::allocate_member_objects()
{
//   cout << "inside Building::allocate_member_objects()" << endl;
   
   hsv_bin_ptr=new vector<quadruple*>;
}		       

void Building::initialize_member_objects()
{
   ID=-1;
   ground_z=POSITIVEINFINITY;
   roof_z=NEGATIVEINFINITY;
   primary_color.first=-1;
   primary_color.second=-1;
   primary_color.third=-1;
   rectangle_sidefaces_map_ptr=NULL;

   for (unsigned int c=0; c<colorfunc::get_n_quantized_colors(); c++)
   {
      color_bin_frequency.push_back(0);
   }
}

Building::Building() 
{
   allocate_member_objects();
   initialize_member_objects();
}

Building::~Building()
{
   delete hsv_bin_ptr;
   delete rectangle_sidefaces_map_ptr;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,Building& B)
{
   outstream << endl;
   outstream << "Building ID = " << B.get_ID() << endl;
   for (unsigned int p=0; p<B.polyhedra_ptrs.size(); p++)
   {
      cout << "Polyhedron p = " << p << endl;
      cout << *(B.polyhedra_ptrs[p]) << endl;
   }
   
   return outstream;
}

// ==========================================================================
// Member function import_from_OFF_files() parses Object File Format
// (OFF) files for the particular building labeled by input ID.  It
// instantiates its polyhedra parts and returns their IDs within an
// output STL vector.

void Building::import_from_OFF_files(int ID,string OFF_subdir)
{
   cout << "inside Building::import_from_OFF_files()" << endl;
   cout << "OFF_subdir = " << OFF_subdir << endl;
   
   this->ID=ID;

   string prefix="building_"+stringfunc::number_to_string(ID)+"_";
   vector<string> building_polyhedra_filenames=
      filefunc::files_in_subdir_matching_substring(OFF_subdir,prefix);

   cout << "building_polyhedra_filenames.size() = "
        << building_polyhedra_filenames.size() << endl;

   delete rectangle_sidefaces_map_ptr;
   rectangle_sidefaces_map_ptr=new RECTANGLE_SIDEFACES_MAP;

   vector<threevector> building_corners;
   for (unsigned int f=0; f<building_polyhedra_filenames.size(); f++)
   {
      cout << "f = " << f
           << " building polyhedron filename = "
           << building_polyhedra_filenames[f] << endl;

      polyhedron* polyhedron_ptr=new polyhedron(f);
      polyhedra_ptrs.push_back(polyhedron_ptr);

      fourvector polyhedron_color;
      polyhedron_ptr->read_OFF_file(
         building_polyhedra_filenames[f],polyhedron_color);

      vector<polygon*> rectangle_ptrs=
         polyhedron_ptr->reconstruct_rectangular_sides_from_OFF_file(
            building_polyhedra_filenames[f]);

      for (unsigned int r=0; r<rectangle_ptrs.size(); r++)
      {
         polygon* rectangle_ptr=rectangle_ptrs[r];
//         cout << "r = " << r 
//              << " rectangular side face = " << *rectangle_ptr
//              << endl;
         twovector rect_poly_IDs(r,f);
         (*rectangle_sidefaces_map_ptr)[rect_poly_IDs]=rectangle_ptr;
      } // loop over index r labeling rectangular side faces
//      outputfunc::enter_continue_char();

// Save minimal [maximal] vertex Z value within ground_z [roof_z]
// member variable:

      unsigned int n_vertices=polyhedron_ptr->get_n_vertices();
      for (unsigned int v=0; v<n_vertices; v++)
      {
         vertex curr_vertex=polyhedron_ptr->get_vertex(v);
         ground_z=basic_math::min(ground_z,curr_vertex.get_posn().get(2));
         roof_z=basic_math::max(roof_z,curr_vertex.get_posn().get(2));
         building_corners.push_back(curr_vertex.get_posn());
      }
   } // loop over index f

   identify_occluded_rectangle_sidefaces();

// Compute COM for building corners lying close to roof_z:

   vector<threevector> building_roof_corners;
   const double distance_from_rooftop=2.5;	// meters
   for (unsigned int v=0; v<building_corners.size(); v++)
   {
      threevector curr_corner(building_corners[v]);
      if (roof_z -curr_corner.get(2) < distance_from_rooftop)
      {
         building_roof_corners.push_back(curr_corner);
      }
   }

   roof_COM=Zero_vector;
   for (unsigned int v=0; v<building_roof_corners.size(); v++)
   {
      roof_COM += building_roof_corners[v];
   }
   roof_COM /= building_roof_corners.size();
   roof_COM.put(2,roof_z);

   cout << "ground_z = " << ground_z << endl;
}

// ---------------------------------------------------------------------
// Member function identify_occluded_rectangle_sidefaces() loops over
// all rectangular side faces within the current building.  For each
// side face, it searches for any others which are anti-parallel.
// This method then computes the distances of the first rectangle's
// corners to the second.  If the distances are all small, the first
// rectangle abuts the second and is completely occluded.  The
// occluded flag for the first rectangle polygon is then set to true.

void Building::identify_occluded_rectangle_sidefaces()
{
//   cout << "inside Building::clean_rectangle_sidefaces()" << endl;
//   cout << "Building ID = " << get_ID() << endl;
//   cout << "rectangle_sidefaces_map.size() = "
//        << rectangle_sidefaces_map_ptr->size() << endl;

   for (RECTANGLE_SIDEFACES_MAP::iterator iter1=rectangle_sidefaces_map_ptr->
           begin(); iter1 != rectangle_sidefaces_map_ptr->end(); iter1++)
   {
      polygon* rectangle1_ptr=iter1->second;
      threevector n1_hat=rectangle1_ptr->get_normal();
      for (RECTANGLE_SIDEFACES_MAP::iterator iter2=
              rectangle_sidefaces_map_ptr->begin();
           iter2 != rectangle_sidefaces_map_ptr->end(); iter2++)
      {
         if (iter2==iter1) continue;
         polygon* rectangle2_ptr=iter2->second;
         threevector n2_hat=rectangle2_ptr->get_normal();

// Ignore any two rectangles which are not anti-parallel:

         double dotproduct=-n1_hat.dot(n2_hat);
//         cout << "dotproduct = " << dotproduct << endl;
         if (dotproduct < 0.99) continue;

// Loop over rectangle 2 corners and compute their distances to
// rectangle 1:

         bool rectangle2_hidden_flag=true;
         const double max_distance_threshold=1;	// meter
         for (unsigned int c=0; c<rectangle2_ptr->get_nvertices(); c++)
         {
            threevector V(rectangle2_ptr->get_vertex(c));
            double curr_distance=rectangle1_ptr->point_dist_to_polygon(V);
//            cout << "c = " << c << " curr_dist = " << curr_distance << endl;
            if (curr_distance > max_distance_threshold)
            {
               rectangle2_hidden_flag=false;

               break;
            }
         } // loop over index c labeling rectangle 2 corners
         
         if (rectangle2_hidden_flag)
         {
//            cout << "Rectangle2 is hidden" << endl;
            rectangle2_ptr->set_occluded_flag(true);
         }

      } // iter2 loop
   } // iter1 loop

}

// ==========================================================================
// Building coloring member functions
// ==========================================================================

// Member function push_back_hsv()

void Building::push_back_hsv(double h,double s,double v)
{
//   cout << "inside Building::push_back_hsv()" << endl;

   int color_bin_number=colorfunc::assign_hsv_to_color_histogram_bin(h,s,v);
   color_bin_frequency[color_bin_number]=
      color_bin_frequency[color_bin_number]+1;

   quadruple* curr_hsv_bin_ptr=new quadruple(h,s,v,color_bin_number);
   hsv_bin_ptr->push_back(curr_hsv_bin_ptr);
}

// ---------------------------------------------------------------------
// Member function compute_primary_color()

bool Building::compute_primary_color()
{
//   cout << "inside Building::compute_primary_color()" << endl;

   const unsigned int min_color_samples=10000;
   if (hsv_bin_ptr->size() < min_color_samples) return false;

   int primary_color_bin_number=-1;
   int max_color_frequency=-1;
   for (unsigned int c=0; c<color_bin_frequency.size(); c++)
   {
      if (color_bin_frequency[c] > max_color_frequency)
      {
         max_color_frequency=color_bin_frequency[c];
         primary_color_bin_number=c;
      }
   }

   double r,g,b;
   vector<double> reds,greens,blues;
   for (unsigned int i=0; i<hsv_bin_ptr->size(); i++)
   {
      quadruple* curr_hsv_bin_ptr(hsv_bin_ptr->at(i));
      int curr_color_bin_number=basic_math::round(curr_hsv_bin_ptr->fourth);
      
      if (curr_color_bin_number != primary_color_bin_number)
      {
         if (curr_color_bin_number==7)	// white
         {
		// Do nothing
         }
         else
         {
            continue;
         }
      }

      colorfunc::hsv_to_RGB(
         curr_hsv_bin_ptr->first,curr_hsv_bin_ptr->second,
         curr_hsv_bin_ptr->third,r,g,b);
      reds.push_back(r);
      greens.push_back(g);
      blues.push_back(b);
   }

   primary_color.first=mathfunc::mean(reds);
   primary_color.second=mathfunc::mean(greens);
   primary_color.third=mathfunc::mean(blues);
   
   cout << "Building ID = " << get_ID() << endl;
   cout << "n_samples = " << hsv_bin_ptr->size() << endl;
   cout << "Primary color: r = " << primary_color.first
        << " g = " << primary_color.second
        << " b = " << primary_color.third << endl;

   return true;
}

/*
// ---------------------------------------------------------------------
// Member function point_inside()

bool Building::point_inside(const threevector& V)
{
   cout << "inside Building::point_inside()" << endl;

   bool point_inside_flag=true;
   double z=V.get(2);
   if (z < ground_z || z > roof_z) point_inside_flag=false;
//   if (!footprint_poly.point_inside_polygon(V)) point_inside_flag=false;

   return point_inside_flag;
}
*/
