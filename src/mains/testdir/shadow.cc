// ==========================================================================
// Program SHADOW
// ==========================================================================
// Last updated on 3/6/06
// ==========================================================================

#include <iomanip>
#include <iostream>
#include <set>
#include <string>
#include <vector>
#include "threeDgraphics/draw3Dfuncs.h"
#include "general/filefuncs.h"
#include "math/fourvector.h"
#include "geometry/plane.h"
#include "general/outputfuncs.h"
#include "general/sysfuncs.h"
#include "math/threevector.h"
#include "image/TwoDarray.h"
#include "threeDgraphics/xyzpfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ios;
using std::ofstream;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

   string xyzp_filename="line.xyzp";
//   string xyzp_filename="surfaces.xyzp";
//   string xyzp_filename="surfaces2.xyzp";

   myvector vertex[4];
   vertex[0]=myvector(2,2,0);
   vertex[1]=myvector(-2,2,0);
   vertex[2]=myvector(-2,-2,0);
   vertex[3]=myvector(2,-2,0);

   linesegment l(vertex[0],vertex[1]);
   polygon zplane(4,vertex);

   vertex[0]=myvector(0,0,0);
   vertex[1]=myvector(0.5,0);
   vertex[2]=myvector(0.5,0,2);
   vertex[3]=myvector(0,0,2);
   polygon yplane(4,vertex);
   
   double du=0.005;
   double dv=0.005;
   draw3Dfunc::ds=min(du,dv);

/*
   filefunc::deletefile(xyzp_filename);
   draw3Dfunc::draw_line(l,xyzp_filename,draw3Dfunc::annotation_value1);
   draw3Dfunc::draw_rectangle_grid(
      zplane,xyzp_filename,draw3Dfunc::annotation_value1,du,dv);
   draw3Dfunc::draw_rectangle_grid(
      yplane,xyzp_filename,draw3Dfunc::annotation_value2,du,dv);
*/

   vector<fourvector>* XYZP_ptr=xyzpfunc::read_xyzp_float_data(xyzp_filename);

//   double minimal_separation=1E-5;
//   double RMS_dist=xyzpfunc::interpoint_RMS_distance(
//      minimal_separation,XYZP_ptr);
//   cout << "RMS distance = " << RMS_dist << endl;

   double theta=50*PI/180;
//   double theta=83*PI/180;
   cout << "Enter illumination angle in degrees:" << endl;
   cin >> theta;
   theta *= PI/180;
   
//   threevector r_hat(cos(theta),0,sin(theta));
   threevector r_hat(0,cos(theta),sin(theta));
   r_hat = -r_hat;

   cout << "r_hat = " << r_hat << endl;

   threevector COM(0,0,0);
//   threevector COM(-100.0*r_hat);
   plane COM_plane(r_hat,COM);
   cout << "a_hat = " << COM_plane.get_ahat() << endl;
   cout << "b_hat = " << COM_plane.get_bhat() << endl;   
   cout << "n_hat = " << COM_plane.get_nhat() << endl;

   vector<threevector>* planar_coords_ptr=COM_plane.coords_wrt_plane(
      *XYZP_ptr);

   double a_max=NEGATIVEINFINITY;
   double b_max=NEGATIVEINFINITY;   
   double n_max=NEGATIVEINFINITY;
   double a_min=POSITIVEINFINITY;
   double b_min=POSITIVEINFINITY;
   double n_min=POSITIVEINFINITY;
   for (int i=0; i<planar_coords_ptr->size(); i++)
   {
      threevector curr_planar_coords( (*planar_coords_ptr)[i] );
      a_max=max(a_max,curr_planar_coords.get(0));
      a_min=min(a_min,curr_planar_coords.get(0));
      b_max=max(b_max,curr_planar_coords.get(1));
      b_min=min(b_min,curr_planar_coords.get(1));
      n_max=max(n_max,curr_planar_coords.get(2));
      n_min=min(n_min,curr_planar_coords.get(2));

//      cout << "i = " << i << " a = " << curr_planar_coords.get(0)
//           << " b = " << curr_planar_coords.get(1)
//           << " n = " << curr_planar_coords.get(2) << endl;
   }
   cout << "a_max = " << a_max << " a_min = " << a_min << endl;
   cout << "b_max = " << b_max << " b_min = " << a_min << endl;
   cout << "n_max = " << n_max << " n_min = " << a_min << endl;

   double da=3*min(du,dv);
   double db=3*min(du,dv);
   int width=basic_math::round((a_max-a_min)/da)+1;
   int height=basic_math::round((b_max-b_min)/db)+1;
  
   cout << "width = " << width << " height = " << height << endl;
    
   double max_factor=1;
   cout << "Enter max surface depth factor:" << endl;
   cin >> max_factor;
   double dn=max_factor*max(da,db);

   TwoDarray<vector<pair<double,int> >* >* depth_buffer_ptr=
      new TwoDarray<vector<pair<double,int> >* >(width,height);

   int overlap=0;
   cout << "Enter overlap:" << endl;
   cin >> overlap;
   
   for (unsigned int i=0; i<planar_coords_ptr->size(); i++)
   {
      if (i%100000==0) cout << i/100000 << " " << flush;

      threevector curr_planar_coords( (*planar_coords_ptr)[i] );
      int pa=basic_math::round((curr_planar_coords.get(0)-a_min)/da);
      int pb=basic_math::round((curr_planar_coords.get(1)-b_min)/db);
//      int pn=basic_math::round((curr_planar_coords.get(2)-n_min)/dn);

//      const int overlap=0;
//      const int overlap=1;
      for (int d_pa=-overlap; d_pa<=overlap; d_pa++)
      {
         int curr_pa=max(0,pa+d_pa);
         curr_pa=min(curr_pa,width-1);
         for (int d_pb=-overlap; d_pb<=overlap; d_pb++)
         {
            int curr_pb=max(0,pb+d_pb);
            curr_pb=min(curr_pb,height-1);
            vector<pair<double,int> >* curr_vector_ptr=
               depth_buffer_ptr->get(curr_pa,curr_pb);
            if (curr_vector_ptr==NULL)
            {
               curr_vector_ptr=new vector<pair<double,int> >;
               depth_buffer_ptr->put(curr_pa,curr_pb,curr_vector_ptr);
            }
            curr_vector_ptr->push_back(pair<double,int>(
               curr_planar_coords.get(2),i));
         } // loop over d_pb index
      } // loop over d_pa index
   } // loop over index i labeling points in cloud
   
   outputfunc::newline();

   for (int pa=0; pa<width; pa++)
   {
      for (int pb=0; pb<height; pb++)
      {
         vector<pair<double,int> >* curr_vector_ptr=
            depth_buffer_ptr->get(pa,pb);
         if (curr_vector_ptr != NULL)
         {
            std::sort(curr_vector_ptr->begin(),curr_vector_ptr->end());
         
            double nearest_depth=(*curr_vector_ptr)[0].first;
            int i_nearest=(*curr_vector_ptr)[0].second;
            for (int j=0; j<curr_vector_ptr->size(); j++)
            {
               double curr_depth=(*curr_vector_ptr)[j].first;
               int i=(*curr_vector_ptr)[j].second;

               if (curr_depth-nearest_depth > dn)
               {
                  (*XYZP_ptr)[i].put(3,0.5);
                  (*XYZP_ptr)[i_nearest].put(3,0.1);
               }
            } // loop over index j labeling entries in (pa,pb) depth vector
            delete curr_vector_ptr;
         } // curr_vector_ptr != NULL conditional
      } // loop over pb index
   } // loop over pa index

   string shadow_xyzp_filename="shadows.xyzp";
   filefunc::deletefile(shadow_xyzp_filename);
   xyzpfunc::write_xyzp_data(shadow_xyzp_filename,XYZP_ptr,false);

// Draw rectangle representing COM plane into output XYZP file:

   vector<threevector> corner;
   corner.push_back(COM_plane.world_coords(twovector(a_max,b_max)));
   corner.push_back(COM_plane.world_coords(twovector(a_max,b_min)));
   corner.push_back(COM_plane.world_coords(twovector(a_min,b_min)));
   corner.push_back(COM_plane.world_coords(twovector(a_min,b_max)));
   polygon COM_poly(corner);

//   cout << "COM_poly = " << COM_poly << endl;
   draw3Dfunc::draw_polygon(COM_poly,shadow_xyzp_filename,0.7);

   delete planar_coords_ptr;
   delete depth_buffer_ptr;
   delete XYZP_ptr;
   
}
