// ==========================================================================
// TRIANGLESGROUP class member function definitions
// ==========================================================================
// Last modified on 12/4/10; 1/26/12; 3/22/14; 4/5/14
// ==========================================================================

#include <iomanip>
#include <osg/Geode>
#include "general/filefuncs.h"
#include "network/Network.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "osg/osgGeometry/Triangle.h"
#include "osg/osgGeometry/TrianglesGroup.h"
#include "geometry/triangles_group.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::ios;
using std::ofstream;
using std::ostream;
using std::setw;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void TrianglesGroup::allocate_member_objects()
{
}		       

void TrianglesGroup::initialize_member_objects()
{
   GraphicalsGroup_name="TrianglesGroup";
   
   size[2]=1.0;
   size[3]=1.0;
   triangles_network_ptr=NULL;
   zsample_twoDarray_ptr=NULL;

   get_OSGgroup_ptr()->setUpdateCallback( 
      new AbstractOSGCallback<TrianglesGroup>(
         this, &TrianglesGroup::update_display));
}		      
 
TrianglesGroup::TrianglesGroup(const int p_ndims,Pass* PI_ptr):
   GraphicalsGroup(p_ndims,PI_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

TrianglesGroup::~TrianglesGroup()
{
   delete triangles_network_ptr;
   delete zsample_twoDarray_ptr;
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const TrianglesGroup& f)
{
   int node_counter=0;
   for (unsigned int n=0; n<f.get_n_Graphicals(); n++)
   {
      Triangle* Triangle_ptr=f.get_Triangle_ptr(n);
      outstream << "Triangle node # " << node_counter++ << endl;
      outstream << "Triangle = " << *Triangle_ptr << endl;
   }
   return(outstream);
}

// ==========================================================================
// Triangle creation and manipulation methods
// ==========================================================================

// Following Vadim's advice on 7/18/05, we separate off member
// function generate_new_Triangle from all other graphical insertion
// and manipulation methods...

Triangle* TrianglesGroup::generate_new_Triangle(int ID)
{
   if (ID==-1) ID=get_next_unused_ID();
   Triangle* curr_Triangle_ptr=new Triangle(ID);
   GraphicalsGroup::insert_Graphical_into_list(curr_Triangle_ptr);
   insert_graphical_PAT_into_OSGsubPAT(curr_Triangle_ptr,Unsigned_Zero);
   return curr_Triangle_ptr;
}

// --------------------------------------------------------------------------
void TrianglesGroup::generate_triangle_geode(Triangle* triangle_ptr)
{
//   cout << "inside TG::generate_triangle_geode()" << endl;
   osg::Geode* geode_ptr=triangle_ptr->generate_drawable_geode();
   triangle_ptr->get_PAT_ptr()->addChild(geode_ptr);
}

// --------------------------------------------------------------------------
// Member function erase_Triangle sets boolean entries within the
// member map coords_erased to true for the current Triangle.  When
// Triangle crosshairs are drawn within
// TrianglesGroup::reassign_PAT_ptrs(), entries within this STL map are
// first checked and their positions are set to large negative values
// to prevent them from appearing within the OSG data window.  Yet the
// Triangle itself continues to exist.

bool TrianglesGroup::erase_Triangle()
{   
   return erase_Graphical();
}

// --------------------------------------------------------------------------
// Member function unerase_Triangle queries the user to enter the ID
// for some erased Triangle.  It then unerases that Triangle within the
// current image.

bool TrianglesGroup::unerase_Triangle()
{   
   bool Triangle_unerased_flag=unerase_Graphical();
   if (Triangle_unerased_flag)
   {
   }
   return Triangle_unerased_flag;
}

/*
// --------------------------------------------------------------------------
void TrianglesGroup::generate_triangles(triangles_group* triangles_group_ptr)
{
   cout << "inside TrianglesGroup::generate_triangles()" << endl;

   for (unsigned int t=0; t<triangles_group_ptr->get_n_triangles(); t++)
   {
      triangle* curr_triangle_ptr=triangles_group_ptr->get_triangle_ptr(t);
      
   } // loop over index t labeling input triangles
   
}
*/

// ==========================================================================
// Ascii file I/O methods
// ==========================================================================

// Member function save_info_to_file loops over all Triangles within
// *get_Trianglelist_ptr() and prints their times, IDs, pass numbers
// UVW vertex coordinates, vertex IDs and triangle color to the output
// ofstream.  This Triangle information can later be read back in via
// member function read_info_from_file.

void TrianglesGroup::save_info_to_file()
{
   outputfunc::write_banner("Saving Triangle information to ascii file:");
//   string output_filename="triangles.txt";
   string output_filename="triangles_"+pass_ptr->get_passname_prefix()+".txt";

   cout << "output_filename = " << output_filename << endl;
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);
   
   outstream.setf(ios::showpoint);
   const int column_width=15;

   vector<threevector> V;
   for (unsigned int imagenumber=get_first_framenumber(); 
        imagenumber <= get_last_framenumber(); imagenumber++)
   {
      double curr_t=static_cast<double>(imagenumber);
      cout << imagenumber << " " << flush;
      for (unsigned int n=0; n<get_n_Graphicals(); n++)
      {
         Triangle* Triangle_ptr=get_Triangle_ptr(n);
         if (Triangle_ptr != NULL)
         {
            instantaneous_obs* curr_obs_ptr=Triangle_ptr->
               get_particular_time_obs(curr_t,get_passnumber());
            if (curr_obs_ptr != NULL)
            {
               if (!Triangle_ptr->get_mask(curr_t,get_passnumber()))
               {
                  if (Triangle_ptr->get_vertices(curr_t,get_passnumber(),V));
                  {
                     outstream << setw(5) << curr_t 
                               << setw(11) <<Triangle_ptr->get_ID() 
                               << setw(3) << get_passnumber();
                     for (unsigned int j=0; j<3; j++)
                     {
                        outstream << setw(column_width) 
                                  << stringfunc::number_to_string(
                                     V[j].get(0),5)
                                  << setw(column_width) 
                                  << stringfunc::number_to_string(
                                     V[j].get(1),5)
                                  << setw(column_width) 
                                  << stringfunc::number_to_string(
                                     V[j].get(2),5)
                                  << setw(column_width) 
                                  << stringfunc::number_to_string(
                                     Triangle_ptr->get_vertex_ID(j));
                     } // loop over index j labeling triangle vertices
                     outstream << setw(column_width) 
                               << Triangle_ptr->get_permanent_color().r()
                               << " " 
                               << Triangle_ptr->get_permanent_color().g()
                               << " " 
                               << Triangle_ptr->get_permanent_color().b()
                               << " " 
                               << Triangle_ptr->get_permanent_color().a()
                               << endl;
                  } // Triangle_ptr->get_vertices conditional
               } // !erased_flag conditional
            } // curr_obs_ptr != NULL conditional
         } // Triangle_ptr != NULL conditional
      } // loop over index n labeling nodes in *get_Trianglelist_ptr()
   } // loop over imagenumber index
   cout << endl;
   filefunc::closefile(output_filename,outstream);
}

// --------------------------------------------------------------------------
// Member function read_info_from_file parses the ascii text file
// generated by member function save_info_to_file().  This method
// regenerates the Triangles within the list based upon the ascii
// text file information.

void TrianglesGroup::read_info_from_file(
   string triangles_filename,vector<double>& curr_time,
   vector<int>& triangle_ID,vector<int>& pass_number,
   vector<threevector>& V0,vector<int>& V0_ID,
   vector<threevector>& V1,vector<int>& V1_ID,
   vector<threevector>& V2,vector<int>& V2_ID,
   vector<colorfunc::Color>& color)
{
   filefunc::ReadInfile(triangles_filename);

   int n_fields=stringfunc::compute_nfields(filefunc::text_line[0]);
   double X[n_fields];

   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      stringfunc::string_to_n_numbers(n_fields,filefunc::text_line[i],X);
      curr_time.push_back(X[0]);
      triangle_ID.push_back(int(X[1]));
      pass_number.push_back(int(X[2]));
      V0.push_back(threevector(X[3],X[4],X[5]));
      V0_ID.push_back(int(X[6]));
      V1.push_back(threevector(X[7],X[8],X[9]));
      V1_ID.push_back(int(X[10]));
      V2.push_back(threevector(X[11],X[12],X[13]));
      V2_ID.push_back(int(X[14]));
      color.push_back(static_cast<colorfunc::Color>(
         basic_math::round(X[15])));
   } // loop over index i labeling ascii file line number
   
//   for (unsigned int i=0; i<curr_time.size(); i++)
//   {
//      cout << "time = " << curr_time[i]
//           << " ID = " << triangle_ID[i]
//           << " pass = " << pass_number[i] << endl;
//      cout << "V0 = " << V0[i].get(0) << "," << V0[i].get(2) << endl;
//      cout << "V1 = " << V1[i].get(0) << "," << V1[i].get(2) << endl;
//      cout << "V2 = " << V2[i].get(0) << "," << V2[i].get(2) << endl;
//      cout << "color = " << color[i] << endl;
//   }
}

// --------------------------------------------------------------------------
// Member function reconstruct_triangles_from_file_info parses the
// triangle information written to an ascii file by member function
// save_info_to_file().  It then destroys all existing line segments
// and instantiates a new set using the input information.  This
// boolean method returns false it it cannot successfully reconstruct
// triangles from the input file information.

bool TrianglesGroup::reconstruct_triangles_from_file_info()
{
//   string input_filename="triangles.txt";
//   cout << "Enter name of file containing triangle information:" << endl;
//   cin >> input_filename;
   string input_filename="triangles_"+pass_ptr->get_passname_prefix()+".txt";
   return reconstruct_triangles_from_file_info(input_filename);
}

bool TrianglesGroup::reconstruct_triangles_from_file_info(
   string input_filename)
{
//   string input_filename="triangles.txt";
//   cout << "Enter name of file containing triangle information:" << endl;
//   cin >> input_filename;
//   string input_filename="triangles_"+pass_ptr->get_passname_prefix()+".txt";

   vector<double> curr_time;
   vector<int> triangle_ID,pass_number;
   vector<threevector> V0,V1,V2;
   vector<int> V0_ID,V1_ID,V2_ID;
   vector<colorfunc::Color> color;

   read_info_from_file(
      input_filename,curr_time,triangle_ID,pass_number,
      V0,V0_ID,V1,V1_ID,V2,V2_ID,color);
   regenerate_triangles(curr_time,triangle_ID,pass_number,
                        V0,V0_ID,V1,V1_ID,V2,V2_ID,color);

   return true;
//   return false;
}

// ==========================================================================
// Triangle network construction methods
// ==========================================================================

// Member function regenerate_triangles takes in triangle vertex
// information which was presumably previously generated as a function
// of time and pass number.  This method dyanmically generates
// triangle objects which exist for all time, fills in their vertex
// information and then refreshes the display with these triangles.

void TrianglesGroup::regenerate_triangles(
   const vector<double>& curr_time,const vector<int>& triangle_ID,
   const vector<int>& pass_number,
   const vector<threevector>& V0,const vector<int>& vertex0_ID,
   const vector<threevector>& V1,const vector<int>& vertex1_ID,
   const vector<threevector>& V2,const vector<int>& vertex2_ID,
   const vector<colorfunc::Color>& color)
{
//   cout << "inside TG::regenerate_triangles()" << endl;

/*
   cout << "curr_time = " << endl;
   templatefunc::printVector(curr_time);
   outputfunc::enter_continue_char();
   cout << "triangle ID = " << endl;
   templatefunc::printVector(triangle_ID);
   outputfunc::enter_continue_char();
   cout << "V0 = " << endl;
   templatefunc::printVector(V0);
   outputfunc::enter_continue_char();
   cout << "V1 = " << endl;
   templatefunc::printVector(V1);
   outputfunc::enter_continue_char();
   cout << "V2 = " << endl;
   templatefunc::printVector(V2);
   outputfunc::enter_continue_char();
   cout << "color = " << endl;
   templatefunc::printVector(color);
*/

// Destroy all existing Triangles before creating a new Triangle list
// from the input ascii file:

   destroy_all_Graphicals();

   for (unsigned int i=0; i<triangle_ID.size(); i++)
   {
      int curr_ID=triangle_ID[i];
      Triangle* curr_Triangle_ptr=get_ID_labeled_Triangle_ptr(curr_ID);
      if (curr_Triangle_ptr == NULL)
      {
         curr_Triangle_ptr=generate_new_Triangle(curr_ID);
         curr_Triangle_ptr->set_vertex_ID(0,vertex0_ID[i]);
         curr_Triangle_ptr->set_vertex_ID(1,vertex1_ID[i]);
         curr_Triangle_ptr->set_vertex_ID(2,vertex2_ID[i]);
         generate_triangle_geode(curr_Triangle_ptr);
      } // curr_Triangle_ptr==NULL conditional
   } // loop over index i labeling entries in triangle_ID STL vector

   for (unsigned int r=0; r<get_n_Graphicals(); r++)
   {
      Triangle* Triangle_ptr=get_Triangle_ptr(r);

// Initialize Triangle's coords_erased flag to true for all images
// within the current pass:
      
      for (unsigned int n=get_first_framenumber(); n <= get_last_framenumber(); n++)
      {
         double curr_t=static_cast<double>(n);
         Triangle_ptr->set_mask(curr_t,get_passnumber(),true);
      }

// Load time, imagenumber, passnumber and vertex information into
// current Triangle.  Set manually_manipulated flag to true and
// coords_erased flag to false for each STL vector entry:

      for (unsigned int i=0; i<curr_time.size(); i++)
      {
         if (triangle_ID[i]==Triangle_ptr->get_ID())
         {
            vertices.clear();
            vertices.push_back(V0[i]);
            vertices.push_back(V1[i]);
            vertices.push_back(V2[i]);
            Triangle_ptr->set_vertices(curr_time[i],pass_number[i],vertices);

            const threevector origin(0,0,0);
            Triangle_ptr->set_UVW_coords(curr_time[i],pass_number[i],origin);
            const osg::Quat trivial_q(0,0,0,1);
            Triangle_ptr->set_quaternion(curr_time[i],pass_number[i],
                                         trivial_q);
            threevector trivial_scale(1,1,1);
            Triangle_ptr->set_scale(curr_time[i],pass_number[i],
                                    trivial_scale);

            Triangle_ptr->set_permanent_color(color[i]);
            Triangle_ptr->set_mask(curr_time[i],pass_number[i],false);
            Triangle_ptr->set_coords_manually_manipulated(
               curr_time[i],pass_number[i]);

// We would like to be able to unerase a Rectangle in an image before
// the first one in which UVW coordinates were saved into the ascii
// file.  So we copy the coordinate values backwards in time from the
// image where the Rectangle first appears:

            unsigned int n=basic_math::round(curr_time[i]);
            for (unsigned int m=0; m<n; m++)
            {
               threevector p;
               if (!Triangle_ptr->get_UVW_coords(m,pass_number[i],p))
               {
                  Triangle_ptr->set_UVW_coords(m,pass_number[i],origin);
               }
            } // loop over index m labeling images before image number n

         } // triangle_ID[i] == Triangle_ptr->get_ID conditional
      } // loop over index i labeling entries in STL time, triangle_ID
        // and UVW vectors
   } // loop index r labeling over Triangles in Trianglelist
}

// --------------------------------------------------------------------------
// Member function generate_triangles_network instantiates a network
// of the triangles contained within the current TrianglesGroup.  It
// performs a brute-force search to determine which triangles share a
// common edge.  Any two which do are neighbors within the triangles
// network.

void TrianglesGroup::generate_triangles_network(
   const vector<int>& triangle_ID)
{
   unsigned int n_triangles=triangle_ID.size();

//   for (unsigned int i=0; i<n_triangles; i++)
//   {
//      cout << "Triangle ID = " << triangle_ID[i]
//           << " v0 = " << vertex0_ID[i]
//           << " v1 = " << vertex1_ID[i]
//           << " v2 = " << vertex2_ID[i] << endl;
//   }
   triangles_network_ptr=new Network<Triangle*>(n_triangles);

   for (unsigned int n=0; n<n_triangles; n++)
   {
      Triangle* Triangle_ptr=get_ID_labeled_Triangle_ptr(triangle_ID[n]);
      triangles_network_ptr->insert_site(n,Site<Triangle*>(Triangle_ptr));

// Check whether current triangle is a neighbor to any other triangles
// already existing within the network:

      for (unsigned int m=0; m<n; m++)
      {
         Triangle* neighbor_Triangle_ptr=get_ID_labeled_Triangle_ptr(
            triangle_ID[m]);
         
         int n_common_vertices=0;

         if (Triangle_ptr->get_vertex_ID(0)==
             neighbor_Triangle_ptr->get_vertex_ID(0)) n_common_vertices++;
         if (Triangle_ptr->get_vertex_ID(0)==
             neighbor_Triangle_ptr->get_vertex_ID(1)) n_common_vertices++;
         if (Triangle_ptr->get_vertex_ID(0)==
             neighbor_Triangle_ptr->get_vertex_ID(2)) n_common_vertices++;
         
         if (Triangle_ptr->get_vertex_ID(1)==
             neighbor_Triangle_ptr->get_vertex_ID(0)) n_common_vertices++;
         if (Triangle_ptr->get_vertex_ID(1)==
             neighbor_Triangle_ptr->get_vertex_ID(1)) n_common_vertices++;
         if (Triangle_ptr->get_vertex_ID(1)==
             neighbor_Triangle_ptr->get_vertex_ID(2)) n_common_vertices++;

         if (Triangle_ptr->get_vertex_ID(2)==
             neighbor_Triangle_ptr->get_vertex_ID(0)) n_common_vertices++;
         if (Triangle_ptr->get_vertex_ID(2)==
             neighbor_Triangle_ptr->get_vertex_ID(1)) n_common_vertices++;
         if (Triangle_ptr->get_vertex_ID(2)==
             neighbor_Triangle_ptr->get_vertex_ID(2)) n_common_vertices++;

         if (n_common_vertices==2)
         {
            triangles_network_ptr->add_symmetric_link(n,m);
         }
      } // loop over index m labeling preceding triangles
   } // loop over index n labeling triangles

//   cout << "*triangles_network_ptr = " << *triangles_network_ptr << endl;
}

// --------------------------------------------------------------------------
// Member function uniquely_color_triangles performs systematic sweeps
// over all triangles within the TrianglesGroup.  It first sets all
// their permanent colors to a uniform value.  It then performs a
// brute force search over every pair of neighboring triangles.  If
// they share the same color, the color index for one of them is
// incremented.  The search continues until every triangle has a
// different color from all its neighbors.

void TrianglesGroup::color_triangles()
{   
//   cout << "inside TrianglesGroup::color_triangles()" << endl;

// Initially set all triangles to color 0:

   int init_color_index=0;
   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      Triangle* Triangle_ptr=get_Triangle_ptr(n);
      Triangle_ptr->set_permanent_color(
         colorfunc::get_color(init_color_index));
   } // loop over index n labeling triangles

   bool valid_coloring_found=false;
   while (!valid_coloring_found)
   {

// Search over all triangles for any neighbors which have the same
// color.  Increment color index of any neighbor with the same color
// as the current triangle:

      valid_coloring_found=true;
      for (unsigned int r=0; r<triangles_network_ptr->size(); r++)
      {
         Site<Triangle*>* curr_triangle_site_ptr=triangles_network_ptr->
            get_site_ptr(r);         
         vector<int> neighbors(curr_triangle_site_ptr->get_neighbors());
         
         Triangle* curr_triangle_ptr=curr_triangle_site_ptr->get_data();
         int curr_color_index=colorfunc::get_colorfunc_color(
            curr_triangle_ptr->get_curr_color());
         for (unsigned int p=0; p<neighbors.size(); p++)
         {
            Triangle* neighbor_triangle_ptr=triangles_network_ptr->
               get_site_data_ptr(neighbors[p]);
            int neighbor_color_index=
               colorfunc::get_colorfunc_color(
                  neighbor_triangle_ptr->get_curr_color());
            if (neighbor_color_index==curr_color_index)
            {
               valid_coloring_found=false;
               neighbor_triangle_ptr->set_permanent_color(
                  colorfunc::get_color(neighbor_color_index+1));  
            }
         } // loop over index p labeling neighbors of current triangle
      } // loop over index r labeling triangle sites
   } // while valid coloring not found 
}

// --------------------------------------------------------------------------
// Member function update_display is repeatedly executed by an
// UpdateCallback in a main program.

void TrianglesGroup::update_display()
{   
//   cout << "inside TG::update_display()" << endl;
   
   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      Triangle* Triangle_ptr=get_Triangle_ptr(n);
      
      if (Triangle_ptr != NULL)
      {
         vertices.clear();
         Triangle_ptr->get_vertices(get_curr_t(),get_passnumber(),vertices);
         Triangle_ptr->set_geom_vertices(vertices);
         Triangle_ptr->update_plane();
         Triangle_ptr->set_curr_color(
            Triangle_ptr->get_permanent_color());
      }
   } // loop over index n labeling triangles
   GraphicalsGroup::update_display();
}

// ==========================================================================
// XY lattice and Z coordinate methods
// ==========================================================================

// Member function extremal_XY_coords loops over every triangle and
// extracts the maximum and minimum X and Y vertex coordinate values.

void TrianglesGroup::extremal_XY_coords(
   double& min_x,double& min_y,double& max_x,double& max_y)
{   
   min_x=min_y=POSITIVEINFINITY;
   max_x=max_y=NEGATIVEINFINITY;

   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      vertices.clear();
      Triangle* Triangle_ptr=get_Triangle_ptr(n);
      Triangle_ptr->get_vertices(get_curr_t(),get_passnumber(),vertices);
      for (unsigned int i=0; i<vertices.size(); i++)
      {
         min_x=basic_math::min(min_x,vertices[i].get(0));
         min_y=basic_math::min(min_y,vertices[i].get(1));
         max_x=basic_math::max(max_x,vertices[i].get(0));
         max_y=basic_math::max(max_y,vertices[i].get(1));
      }
   } // loop over index n labeling triangles
}

// ---------------------------------------------------------------------
// Member function sample_zcoords_on_XYgrid instantiates member
// twoDarray *zsample_twoDarray_ptr and determines its extremal X and
// Y limits.  It then samples the triangle network's z-values at (X,Y)
// sites within the rectangular XY lattice.  The sampled z coordinates
// are saved within *zsample_twoDarray_ptr.

void TrianglesGroup::sample_zcoords_on_XYgrid()
{
   const int nx_bins=100;
   const int ny_bins=100;

/*
   int nx_bins,ny_bins;
   cout << "Enter nx bins:" << endl;
   cin >> nx_bins;
   cout << "Enter ny bins:" << endl;
   cin >> ny_bins;
*/
 
   double min_x,min_y,max_x,max_y;
   extremal_XY_coords(min_x,min_y,max_x,max_y);
   double dx=(max_x-min_x)/(nx_bins-1);
   double dy=(max_y-min_y)/(ny_bins-1);
   
//   cout << "min_x = " << min_x << " max_x = " << max_x 
//        << " dx = " << dx << endl;
//   cout << "min_y = " << min_y << " max_y = " << max_y 
//        << " dy = " << dy << endl;
   
   delete zsample_twoDarray_ptr;
   zsample_twoDarray_ptr=new twoDarray(nx_bins,ny_bins);
   zsample_twoDarray_ptr->set_xlo(min_x);
   zsample_twoDarray_ptr->set_xhi(max_x);
   zsample_twoDarray_ptr->set_deltax(dx);
   zsample_twoDarray_ptr->set_ylo(min_y);
   zsample_twoDarray_ptr->set_yhi(max_y);
   zsample_twoDarray_ptr->set_deltay(dy);

   threevector p_proj;
   for (unsigned int px=0; px<zsample_twoDarray_ptr->get_mdim(); px++)
   {
      for (unsigned int py=0; py<zsample_twoDarray_ptr->get_ndim(); py++)
      {

// Default sentinel NEGATIVEINFINITY value marks sample (x,y)
// locations which do not project onto any triangle within the
// Delaunay network:

         zsample_twoDarray_ptr->put(px,py,NEGATIVEINFINITY);

         double x,y;
         zsample_twoDarray_ptr->pixel_to_point(px,py,x,y);
         for (unsigned int n=0; n<get_n_Graphicals(); n++)
         {
            if (get_Triangle_ptr(n)->z_projection(x,y,p_proj))
            {
               zsample_twoDarray_ptr->put(px,py,p_proj.get(2));
               break;
            }
         } // loop over index n labeling triangles in TriangleGroup
      } // loop over py index
   } // loop over px index
//   cout << "*zsample_twoDarray_ptr = " << *zsample_twoDarray_ptr << endl;
}


