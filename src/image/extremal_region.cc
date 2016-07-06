// =========================================================================
// Extremal_Region class member function definitions
// =========================================================================
// Last modified on 10/8/12; 10/13/12; 10/16/12; 4/5/14
// =========================================================================

#include <iostream>
#include <math.h>
#include "math/basic_math.h"
#include "image/extremal_region.h"
#include "math/genmatrix.h"
#include "image/graphicsfuncs.h"
#include "math/mathfuncs.h"
#include "general/outputfuncs.h"

using std::cout;
using std::endl;
using std::flush;
using std::ostream;
using std::pair;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void extremal_region::allocate_member_objects()
{
   pixel_map_ptr=new PIXEL_MAP;
   prob_map_ptr=new PROB_MAP;
}

void extremal_region::initialize_member_objects()
{
   bright_region_flag=true;
   visited_flag=false;
   pixel_area=pixel_perim=0;
   min_px=min_py=POSITIVEINFINITY;
   max_px=max_py=NEGATIVEINFINITY;
   min_reasonable_pixel_width=2;
   min_reasonable_pixel_height=4;
   Euler_number=0;
   n_horiz_crossings=-1;
   entropy=-1;
   px_sum=sqr_px_sum=0;
   py_sum=sqr_py_sum=0;
   px_py_sum=0;
   cube_px_sum=sqr_px_py_sum=sqr_py_px_sum=cube_py_sum=0;
   z_sum=sqr_z_sum=cube_z_sum=quartic_z_sum=0;
   bbox_polygon_ptr=NULL;
}		 

// ---------------------------------------------------------------------
extremal_region::extremal_region()
{
   allocate_member_objects();
   initialize_member_objects();
   ID=-1;
}

// ---------------------------------------------------------------------
extremal_region::extremal_region(int id)
{
   allocate_member_objects();
   initialize_member_objects();
   ID=id;
}

// ---------------------------------------------------------------------
// Copy constructor:

extremal_region::extremal_region(const extremal_region& er)
{
   docopy(er);
}

extremal_region::~extremal_region()
{
//    cout << "inside extremal_region destructor" << endl;
    delete pixel_map_ptr;
    delete prob_map_ptr;
}

// ---------------------------------------------------------------------
void extremal_region::docopy(const extremal_region& er)
{
}

// Overload = operator:

extremal_region& extremal_region::operator= (const extremal_region& er)
{
   if (this==&er) return *this;
   docopy(er);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const extremal_region& er)
{
   outstream << endl;

   outstream << "ID = " << er.get_ID() << endl;
   outstream << "pixel area = " << er.pixel_area << endl;
   outstream << "pixel perim = " << er.pixel_perim << endl;
   outstream << "min_px = " << er.min_px << endl;
   outstream << "max_px = " << er.max_px << endl;
   outstream << "min_py = " << er.min_py << endl;
   outstream << "max_py = " << er.max_py << endl;
   outstream << "Euler_number = " << er.Euler_number << endl;

   outstream << "aspect_ratio = " << er.get_aspect_ratio() << endl;
   outstream << "compactness = " << er.get_compactness() << endl;
   outstream << "n_holes = " << er.get_n_holes() << endl;
   outstream << "n_horiz_crossings = " << er.get_n_horiz_crossings() << endl;

   return outstream;
}

// =========================================================================
// Set & get member functions
// =========================================================================

void extremal_region::set_image_height(unsigned int h)
{
   image_height=h;

//   cout << "inside extremal_region::set_image_height(), image_height = "
//        << image_height << endl;
   horiz_crossings.reserve(image_height);
   horiz_crossings.clear();
   for (unsigned int r=0; r<image_height; r++)
   {
      horiz_crossings.push_back(0);
   }
}

void extremal_region::set_bbox(unsigned int min_px,unsigned int min_py,
                               unsigned int max_px,unsigned int max_py)
{
//   cout << "inside extremal_region::set_bbox()" << endl;
   this->min_px=min_px;
   this->max_px=max_px;
   this->min_py=min_py;
   this->max_py=max_py;
//   cout << "min_px = " << min_px << " max_px = " << max_px
//        << " min_py = " << min_py << " max_py = " << max_py << endl;
}

void extremal_region::get_bbox(
   unsigned int& min_px,unsigned int& min_py,
   unsigned int& max_px,unsigned int& max_py) 
{
   min_px=this->min_px;
   max_px=this->max_px;
   min_py=this->min_py;
   max_py=this->max_py;
//   cout << "min_px = " << min_px << " max_px = " << max_px 
//        << " min_py = " << min_py << " max_py = " << max_py << endl;
}

double extremal_region::get_aspect_ratio() const
{
//   cout << "inside extremal_region::get_aspect_ratio()" << endl;
//   cout << "max_px = " << max_px << " min_px = " << min_px << endl;
//   cout << "max_py = " << max_py << " min_py = " << min_py << endl;
//   double aspect_ratio=double(max_px-min_px+1)/double(max_py-min_py+1);
   double aspect_ratio=double(get_pixel_width())/double(get_pixel_height());
//   cout << "aspect_ratio = " << aspect_ratio << endl;
   return aspect_ratio;
}

double extremal_region::get_compactness() const
{
   double compactness=sqrt(pixel_area)/pixel_perim;
   return compactness;
}

int extremal_region::get_n_holes() const
{

// Recall Euler number = # connected components - # holes
// By definition, extremal region has exactly one connected component

   int n_holes=1-Euler_number;
   return n_holes;
}

void extremal_region::set_RLE_pixel_IDs(const vector<int>& RLE_pixel_IDs)
{
   for (unsigned int i=0; i<RLE_pixel_IDs.size(); i++)
   {
      this->RLE_pixel_IDs.push_back(RLE_pixel_IDs[i]);
   }
}

// =========================================================================
// Property manipulation member functions
// =========================================================================

void extremal_region::update_bbox(unsigned int px,unsigned int py)
{
//   cout << "inside extremal_region::update_bbox(), px = " << px 
//        << " py = " << py << endl;
//   cout << "Initially, min_px = " << min_px << " max_px = " << max_px 
//        << " min_py = " << max_py << " max_py = " << max_py 
//        << endl;
   min_px=basic_math::min(min_px,px);
   min_py=basic_math::min(min_py,py);
   max_px=basic_math::max(max_px,px);
   max_py=basic_math::max(max_py,py);
}

void extremal_region::update_bbox(
   unsigned int min_px,unsigned int min_py,
   unsigned int max_px,unsigned int max_py)
{
   this->min_px=basic_math::min(min_px,this->min_px);
   this->min_py=basic_math::min(min_py,this->min_py);
   this->max_px=basic_math::max(max_px,this->max_px);
   this->max_py=basic_math::max(max_py,this->max_py);
}

void extremal_region::update_bbox(
   extremal_region* neighbor_extremal_region_ptr)
{
   unsigned int min_neighbor_px,min_neighbor_py,max_neighbor_px,
      max_neighbor_py;
   neighbor_extremal_region_ptr->get_bbox(
      min_neighbor_px,min_neighbor_py,max_neighbor_px,max_neighbor_py);
      
   this->min_px=basic_math::min(min_neighbor_px,this->min_px);
   this->min_py=basic_math::min(min_neighbor_py,this->min_py);
   this->max_px=basic_math::max(max_neighbor_px,this->max_px);
   this->max_py=basic_math::max(max_neighbor_py,this->max_py);

//   cout << "min_neighbor_px = " << min_neighbor_px 
//        << " max_neighbor_px = " << max_neighbor_px << endl;
//   cout << "min_neighbor_py = " << min_neighbor_py
//        << " max_neighbor_py = " << max_neighbor_py << endl;
//   cout << "Finally:" << endl;
//   cout << "this->min_px = " << this->min_px
//        << " this->max_px = " << this->max_px << endl;
//   cout << "this->min_py = " << this->min_py
//        << " this->max_py = " << this->max_py << endl;
}

// =========================================================================
// Moments member functions
// =========================================================================

void extremal_region::update_XY_moments(unsigned int px,unsigned int py)
{
   double curr_sqr_px=sqr(px);
   double curr_sqr_py=sqr(py);

   px_sum += px;
   sqr_px_sum += curr_sqr_px;
   cube_px_sum += px*curr_sqr_px;
//   if (cube_px_sum < 0) 
//   {
//      cout << "inside extremal_region::update_XY_moments()" << endl;
//      cout << "px = " << px << " curr_sqr_px = " << curr_sqr_px << endl;
//      cout << "cube_px_sum = " << cube_px_sum << endl;
//      outputfunc::enter_continue_char();
//   }

   py_sum += py;
   sqr_py_sum += curr_sqr_py;
   cube_py_sum += py*curr_sqr_py;

   px_py_sum += px*py;
   sqr_px_py_sum += curr_sqr_px*py;
   sqr_py_px_sum += curr_sqr_py*px;
}

void extremal_region::update_Z_moments(double z)
{
   double curr_z_sqr=z*z;
   
   z_sum += z;
   sqr_z_sum += z*z;
   cube_z_sum += curr_z_sqr*z;
   quartic_z_sum += curr_z_sqr*curr_z_sqr;

//   cout << "inside update_Z_moments, z = " << z 
//        << " z_sum = " << z_sum 
//        << " sqr_z_sum = " << sqr_z_sum << endl;
}

double extremal_region::get_sigma_px() const
{
//   cout << "inside extremal_region::get_sigma_px()" << endl;
//   cout << "<px**2> = " << get_mean_sqr_px() << endl;
//   cout << "<px>**2 = " << sqr(get_mean_px()) << endl;
//   cout << "pixel_area = " << pixel_area << endl;
   return sqrt(get_mean_sqr_px()-sqr(get_mean_px()));
}

double extremal_region::get_sigma_py() const
{
//   cout << "inside extremal_region::get_sigma_py()" << endl;
//   cout << "<py**2> = " << get_mean_sqr_py() << endl;
//   cout << "<py>**2 = " << sqr(get_mean_py()) << endl;
//   cout << "pixel_area = " << pixel_area << endl;
   return sqrt(get_mean_sqr_py()-sqr(get_mean_py()));
}

double extremal_region::get_dimensionless_px_py_covar() const
{
//   cout << "inside extremal_region::get_dimensionless_px_py_covar()"
//        << endl;
   double numer=get_mean_px_py() - get_mean_px() * get_mean_py();
   double denom=get_sigma_px() * get_sigma_py();
   return numer/denom;
}

void extremal_region::compute_covariance_matrix(twoDarray* cc_twoDarray_ptr)
{
   cout << "inside extremal_region::compute_covariance_matrix()" << endl;
   genmatrix covariance(2,2),covar_inv(2,2);
   genmatrix L(2,2),Linv(2,2);
   
   covariance.put(0,0,get_mean_sqr_px()-sqr(get_mean_px()));
   covariance.put(1,1,get_mean_sqr_py()-sqr(get_mean_py()));
   covariance.put(0,1,get_mean_px_py() - get_mean_px() * get_mean_py());
   covariance.put(1,0,covariance.get(0,1));
   cout << "covariance = " << covariance << endl;

   covariance.two_inverse(covar_inv);
   cout << "covar_inv = " << covar_inv << endl;

   twovector mu(get_mean_px(),get_mean_py());
   cout << "mu = " << mu << endl;

   if (!covariance.cholesky_decomposition(L)) return;

   L.two_inverse(Linv);
//   cout << "Cholesky L = " << L << endl;
//   cout << "Linv = " << Linv << endl;
//   cout << "L*Linv = " << L*Linv << endl;
//   cout << "covar-L*Ltrans = "
//        << covariance - L*L.transpose() << endl;

   int counter=0;
   double qx_sum=0;
   double qy_sum=0;
   vector<twovector> q_transformed;
   double scale=1;
   for (unsigned int py=min_py; py<=max_py; py++)
   {
      for (unsigned int px=min_px; px<=max_px; px++)
      {
         if (int(cc_twoDarray_ptr->get(px,py))==ID)
         {
            twovector trans_x(px-get_mean_px(),py-get_mean_py());
            twovector xhat=1.0/(2*scale)*Linv*trans_x;
            q_transformed.push_back(xhat);
            qx_sum += xhat.get(0);
            qy_sum += xhat.get(1);
            counter++;
         }
      } // loop over px index
   } // loop over py index

   qx_sum /= counter;
   qy_sum /= counter;
//   cout << "qx_sum = " << qx_sum << " qy_sum = " << qy_sum << endl;
//   cout << "counter = " << counter << " pixel_area = " << get_pixel_area()
//        << endl;
   twovector origin(qx_sum,qy_sum);

   double Ixx,Iyy,Ixy,Imin,Imax;
   mathfunc::moment_of_inertia_2D(
      origin,Ixx,Iyy,Ixy,Imin,Imax,q_transformed);

//   cout << "Ixx = " << Ixx << " Iyy = " << Iyy << " Ixy = " << Ixy << endl;
//   cout << "Imin = " << Imin << " Imax = " << Imax << endl;

}

double extremal_region::get_sigma_z() const
{
//   cout << "inside extremal_region::get_sigma_z()" << endl;
//   cout << "<z**2> = " << get_mean_sqr_z() << endl;
//   cout << "<z>**2 = " << sqr(get_mean_z()) << endl;
//   cout << "pixel_area = " << pixel_area << endl;
   return sqrt(get_mean_sqr_z()-sqr(get_mean_z()));
}



double extremal_region::get_skew_px() const
{
   double mu=get_mean_px();
   double sigma=get_sigma_px();
   
   double numer=get_mean_cube_px()-3*mu*sqr(sigma)-mu*sqr(mu);
   double denom=sqr(sigma)*sigma;
   return numer/denom;
}

double extremal_region::get_dimensionless_sqr_px_py() const
{
   double mu_x=get_mean_px();
   double sigma_x=get_sigma_px();
   double mu_y=get_mean_py();
   double sigma_y=get_sigma_py();
   
   double numer=get_mean_sqr_px_py()-mu_y*get_mean_sqr_px()
      -2*mu_x*get_mean_px_py()+2*sqr(mu_x)*mu_y;
   
   double denom=sqr(sigma_x)*sigma_y;
   return numer/denom;
}

double extremal_region::get_dimensionless_sqr_py_px() const
{
   double mu_x=get_mean_px();
   double sigma_x=get_sigma_px();
   double mu_y=get_mean_py();
   double sigma_y=get_sigma_py();
   
   double numer=get_mean_sqr_py_px()-mu_x*get_mean_sqr_py()
      -2*mu_y*get_mean_px_py()+2*sqr(mu_y)*mu_x;
   
   double denom=sqr(sigma_y)*sigma_x;
   return numer/denom;
}


double extremal_region::get_skew_py() const
{
   double mu=get_mean_py();
   double sigma=get_sigma_py();
   
   double numer=get_mean_cube_py()-3*mu*sqr(sigma)-mu*sqr(mu);
   double denom=sqr(sigma)*sigma;
   return numer/denom;
}

double extremal_region::get_skew_z() const
{
   double mu=get_mean_z();
   double sigma=get_sigma_z();

   double numer=get_mean_cube_z()-3*mu*sqr(sigma)-mu*sqr(mu);
   double denom=sqr(sigma)*sigma;
   double skew_z=0;
   if (denom > 0)
   {
      skew_z=numer/denom;
   }
   return skew_z;
}

double extremal_region::get_dimensionless_quartic_z() const
{
   double mu=get_mean_z();
   double sigma=get_sigma_z();

   double numer=get_mean_quartic_z()-4*mu*get_mean_cube_z()
      +6*sqr(mu)*get_mean_sqr_z()-3*sqr(mu)*sqr(mu);

      
   double denom=sqr(sigma)*sqr(sigma);
   double fz=0;
   if (denom > 0)
   {
      fz=numer/denom;
   }
   return fz;
}

// =========================================================================
// Probability member functions
// =========================================================================

void extremal_region::set_object_prob(double p)
{
   set_object_prob(0,p);
}

double extremal_region::get_object_prob() 
{
//   cout << "inside extremal_region::get_object_prob()" << endl;
   return get_object_prob(0);
}

void extremal_region::set_object_prob(int i,double p)
{

   prob_map_iter=prob_map_ptr->find(i);
   if (prob_map_iter==prob_map_ptr->end())
   {
      (*prob_map_ptr)[i]=p;
   }
   else
   {
      prob_map_iter->second=p;
   }
/*
   if (p > 0 && i > 0)
   {
      cout << "inside extremal_region::set_object_prob(i,p) with i = " << i
           << " and p = " << p << endl;
      cout << "get_object_prob(i) = " << get_object_prob(i) << endl;
      outputfunc::enter_continue_char();
   }
*/
}

double extremal_region::get_object_prob(int i) 
{
   prob_map_iter=prob_map_ptr->find(i);
   if (prob_map_iter==prob_map_ptr->end())
   {
      return NEGATIVEINFINITY; 
	// sentinel indicating no previously calculated probability
   }
   else
   {
      return prob_map_iter->second;
   }
}

// =========================================================================
// Pixel member functions
// =========================================================================

int extremal_region::get_n_pixel_IDs() const
{
   return pixel_map_ptr->size();
}

// ---------------------------------------------------------------------
void extremal_region::insert_pixel(int pixel_ID)
{
   pixel_map_iter=pixel_map_ptr->find(pixel_ID);
   if (pixel_map_iter==pixel_map_ptr->end())
   {
      (*pixel_map_ptr)[pixel_ID]=1;
   }
}

void extremal_region::insert_pixels(const vector<int>& input_pixel_IDs)
{
   for (unsigned int i=0; i<input_pixel_IDs.size(); i++)
   {
      insert_pixel(input_pixel_IDs[i]);
   }
}

// ---------------------------------------------------------------------
void extremal_region::print_pixel_IDs(int image_width)
{
   cout << "inside extremal_region::print_pixel_IDs()" << endl;
   cout << "this = " << this << endl;
   cout << "pixel_map.size() = " << pixel_map_ptr->size() << endl;

   unsigned int px_max=NEGATIVEINFINITY;
   unsigned int py_max=NEGATIVEINFINITY;
   unsigned int px_min=POSITIVEINFINITY;
   unsigned int py_min=POSITIVEINFINITY;

   for (pixel_map_iter=pixel_map_ptr->begin(); pixel_map_iter !=
           pixel_map_ptr->end(); pixel_map_iter++)
   {
      int pixel_ID=pixel_map_iter->first;
      if (image_width > 0)
      {
         unsigned int px,py;
         graphicsfunc::get_pixel_px_py(
            pixel_ID,(unsigned int) image_width,px,py);
         cout << "  pixel_ID = " << pixel_ID 
              << " px = " << px << " py = " << py << endl;
         px_max=basic_math::max(px_max,px);
         py_max=basic_math::max(py_max,py);
         px_min=basic_math::min(px_min,px);
         py_min=basic_math::min(py_min,py);
         
      }
      else
      {
         cout << "  pixel_ID = " << pixel_ID << endl;
      }
   }

   cout << "min_px = " << min_px << " max_px = " << max_px
        << " min_py = " << min_py << " max_py = " << max_py << endl;

   if ( (px_max != max_px) || (py_max != max_py) ||
        (px_min != min_px) || (py_min != min_py))
   {
      cout << "MISMATCH!!!" << endl;
      outputfunc::enter_continue_char();
   }
   
}

// ---------------------------------------------------------------------
int extremal_region::reset_pixel_iterator() 
{
   pixel_map_iter=pixel_map_ptr->begin();
   if (pixel_map_iter==pixel_map_ptr->end())
   {
      return -1;
   }
   else
   {
      return pixel_map_iter->first;
   }
}

int extremal_region::get_next_pixel_ID() 
{
//   cout << "inside extremal_region::get_next_pixel_ID()" << endl;
   pixel_map_iter++;
   if (pixel_map_iter==pixel_map_ptr->end())
   {
//      cout << "Returning -1" << endl;
      return -1;
   }
   else
   {
//      cout << "Returning pixel_map_iter->first = "
//           << pixel_map_iter->first << endl;
      return pixel_map_iter->first;
   }
}

// =========================================================================
// Text character detection member functions
// =========================================================================

// Member function region_too_small_or_too_big() rejects the current
// extremal region if its pixel height or width are less or more
// than reasonable threshold values.

bool extremal_region::region_too_small_or_too_big(
   double max_reasonable_pixel_width,double max_reasonable_pixel_height)
{
//   cout << "inside extremal_region::region_too_small()" << endl;
//   cout << "width = " << get_pixel_width()
//        << " height = " << get_pixel_height() << endl;
   
   if (get_pixel_width() < min_reasonable_pixel_width ||
       get_pixel_height() < min_reasonable_pixel_height ||
       get_pixel_width() > max_reasonable_pixel_width ||
       get_pixel_height() > max_reasonable_pixel_height)
   {
      return true;
   }
   else
   {
      return false;
   }
}

// ---------------------------------------------------------------------
// Member function compute_shape_text_prob() takes in a pointer to a
// pretrained probabilistic classifier function generated by Davis
// King's DLIB library.  It computes the probability that the
// shape for the current extremal region corresponds to a text
// character.  If the shape probability is less than the threshold
// specified below, this boolean method returns false.

bool extremal_region::compute_shape_text_prob(
   SHAPES_PFUNCT_TYPE* shapes_pfunct_ptr,
   double shapes_prob_threshold,int object_ID,bool print_flag)
{
   shapes_sample(0)=get_aspect_ratio();
   shapes_sample(1)=get_compactness();
   shapes_sample(2)=get_n_holes();
   shapes_sample(3)=n_horiz_crossings;

   shapes_sample(4)=get_sigma_px()/get_pixel_width();
   shapes_sample(5)=get_sigma_py()/get_pixel_height();
   shapes_sample(6)=get_dimensionless_px_py_covar();

   shapes_sample(7)=get_skew_px();
   shapes_sample(8)=get_dimensionless_sqr_px_py();
   shapes_sample(9)=get_dimensionless_sqr_py_px();
   shapes_sample(10)=get_skew_py();

   double shapes_text_prob=(*shapes_pfunct_ptr)(shapes_sample);
//   cout << " shapes_text_prob = " << shapes_text_prob << endl << endl;

/*
   if (print_flag)
   {
      for (unsigned int i=0; i<11; i++)
      {
         cout << "i = " << i << " shapes_sample[i] = "
              << shapes_sample(i) << endl;
      }
      cout << "shapes_text_prob = " << shapes_text_prob << endl;
   }
*/

// Reject current extremal region if its shape text probability is 
// too small:

   if (shapes_text_prob < shapes_prob_threshold)
   {
      set_object_prob(object_ID,-1);
      return false;
   }
   else
   {
      set_object_prob(object_ID,shapes_text_prob);
//      cout << "object_ID = " << object_ID
//           << " ER ID = " << get_ID()
//           << " shapes_prob = " << shapes_text_prob << endl;
      return true;
   }
}

/*
void extremal_region::append_horiz_crossings(int r,int h_cross) 
{
//   cout << "inside extremal_region::append_horiz_crossings()" << endl;
//   cout << "this = " << this << endl;
//   cout << "horiz_crossings.size() = " << horiz_crossings.size() << endl;
   horiz_crossings[r] += h_cross;
}
*/

// =========================================================================
// Run-length encoding member functions
// =========================================================================

// Method run_length_encode() takes in integer-valued *cc_twoDarray.
// It encodes each run of 1-pixels by its starting and ending pixels'
// IDs.  This method follows the RLE conventions spelled out in
// section 2.3.6 of "Computer and Robot Vision" by Haralick and
// Shapiro (TA 1632.H37 vol 1, 1992). 

void extremal_region::run_length_encode(const twoDarray* cc_twoDarray_ptr)
{
//   cout << "inside extremal_region::run_length_encode()" << endl;
//   cout << "min_px = " << min_px << " max_px = " << max_px << endl;
//   cout << "min_py = " << min_py << " max_py = " << max_py << endl;
   
   int xdim=cc_twoDarray_ptr->get_xdim();

   for (unsigned int py=min_py; py<=max_py; py++)
   {
      bool running=false;
      int start_ID=-1;
      int stop_ID=-1;
      for (unsigned int px=min_px; px<=max_px; px++)
      {
         if (int(cc_twoDarray_ptr->get(px,py))==ID)
         {
            if (!running)
            {
               running=true;
               start_ID=graphicsfunc::get_pixel_ID(px,py,xdim);
            }
         }
         else
         {
            if (running)
            {
               stop_ID=graphicsfunc::get_pixel_ID(px-1,py,xdim);
               running=false;
               RLE_pixel_IDs.push_back(start_ID);
               RLE_pixel_IDs.push_back(stop_ID);
            }
         }
      } // loop over px index

// Terminate any runs which reach end of binary image's current row:

      if (running)
      {
         stop_ID=graphicsfunc::get_pixel_ID(max_px,py,xdim);
         RLE_pixel_IDs.push_back(start_ID);
         RLE_pixel_IDs.push_back(stop_ID);
      }
   } // loop over py index 

//   cout << "ID = " << get_ID() << endl;
   for (unsigned int i=0; i<RLE_pixel_IDs.size(); i += 2)
   {
//      cout << RLE_pixel_IDs[i] << "  " << RLE_pixel_IDs[i+1] << endl;
   }

}

// ---------------------------------------------------------------------
// Member function run_length_decode() converts the 1-pixel runs stored
// within STL vector member RLE_pixel_IDs into pixel locations.  It
// resets the values of those pixels within *cc_twoDarray_ptr equal to
// the current extremal region's ID.

void extremal_region::run_length_decode(
   twoDarray* cc_twoDarray_ptr,double output_value)
{
//   cout << 
//      "inside extremal_region::run_length_decode(), extremal_region ID = " 
//        << ID << endl;
//   cout << "cc_twoDarray_ptr = " << cc_twoDarray_ptr << endl;
//   cout << "output_value = " << output_value << endl;

   int xdim=cc_twoDarray_ptr->get_xdim();
   if (output_value < 0) output_value=ID;

   int pixel_counter=0;
   unsigned int px_start,px_stop,py_start,py_stop;
   for (unsigned int i=0; i<RLE_pixel_IDs.size(); i += 2)
   {
      unsigned int start_pixel_ID=RLE_pixel_IDs[i];
      unsigned int stop_pixel_ID=RLE_pixel_IDs[i+1];
      graphicsfunc::get_pixel_px_py(start_pixel_ID,xdim,px_start,py_start);
      graphicsfunc::get_pixel_px_py(stop_pixel_ID,xdim,px_stop,py_stop);

//      cout << "px_start = " << px_start << " px_stop = " << px_stop
//           << " py_start = " << py_start << " py_stop = " << py_stop
//           << endl;
      
      for (unsigned int px=px_start; px<=px_stop; px++)
      {
         cc_twoDarray_ptr->put(px,py_start,output_value);
         pixel_counter++;
      }
   } // loop over index i labeling RLE runs

//   cout << "*this = " << *this << endl;
//   cout << "pixel_counter = " << pixel_counter << endl;
//   outputfunc::enter_continue_char();
}

// ---------------------------------------------------------------------
// This overloaded version of run_length_decode() returns an STL
// vector containing pixel coordinates for all pixels within all
// run-length-encoded regions.

vector<pair<int,int> > extremal_region::run_length_decode(int xdim)
{
//   cout << 
//      "inside extremal_region::run_length_decode(), extremal_region ID = " 
//        << ID << endl;

   vector<pair<int,int> > decoded_pixels;
   unsigned int px_start,px_stop,py_start,py_stop;
   for (unsigned int i=0; i<RLE_pixel_IDs.size(); i += 2)
   {
      unsigned int start_pixel_ID=RLE_pixel_IDs[i];
      unsigned int stop_pixel_ID=RLE_pixel_IDs[i+1];
      graphicsfunc::get_pixel_px_py(start_pixel_ID,xdim,px_start,py_start);
      graphicsfunc::get_pixel_px_py(stop_pixel_ID,xdim,px_stop,py_stop);

//      cout << "px_start = " << px_start << " px_stop = " << px_stop
//           << " py_start = " << py_start << " py_stop = " << py_stop
//           << endl;
      
      for (unsigned int px=px_start; px<=px_stop; px++)
      {
         pair<int,int> P(px,py_start);
         decoded_pixels.push_back(P);
      }
   } // loop over index i labeling RLE runs

   return decoded_pixels;
}

// ---------------------------------------------------------------------
void extremal_region::compute_bbox_from_RLE_pixels(unsigned int xdim)
{
//   cout << "inside extremal_region::compute_bbox_from_RLE_pixels()"
// 	  << endl;

   unsigned int px,py;
   for (unsigned int i=0; i<RLE_pixel_IDs.size(); i++)
   {
      graphicsfunc::get_pixel_px_py(RLE_pixel_IDs[i],xdim,px,py);
      update_bbox(px,py);
   }
}

// ---------------------------------------------------------------------
// Member function compute_border_pixels() takes in a border_thickness
// parameter which is measured in pixels.  Looping over all entries
// within RLE_pixel_IDs, it sets the corresponding
// pixel value within *cc_twoDarray_ptr equal to cc_value.  It also
// sets all neighboring pixels whose values do not already equal
// cc_value to border_value.  This method returns an STL vector
// containing the extremal region's border pixel IDs.  Border pixels
// lie outside the extremal region.  

vector<pair<int,int> >& extremal_region::compute_border_pixels(
   int border_thickness,twoDarray* cc_twoDarray_ptr)
{
//   cout << "inside extremal_region::compute_border_pixels()" << endl;
   
   unsigned int xdim=cc_twoDarray_ptr->get_xdim();
   unsigned int ydim=cc_twoDarray_ptr->get_ydim();
      
   const double null_value=-1;
   cc_twoDarray_ptr->initialize_values(null_value);

   vector<pair<int,int> > pixel_coords=run_length_decode(xdim);
//   cout << "pixel_coords.size() = " << pixel_coords.size() << endl;

//   unsigned int px,py;
//   int qx,qy;
   const double cc_value=1;
   const double border_value=0;
   for (unsigned int i=0; i<pixel_coords.size(); i++)
   {
      unsigned int px=pixel_coords[i].first;
      unsigned int py=pixel_coords[i].second;
      cc_twoDarray_ptr->put(px,py,cc_value);

      unsigned int qx_lo=basic_math::max(
         px-border_thickness,Unsigned_Zero);
      unsigned int qx_hi=basic_math::min(
         px+border_thickness,xdim-border_thickness);
      unsigned int qy_lo=basic_math::max(
         py-border_thickness,Unsigned_Zero);
      unsigned int qy_hi=basic_math::min(
         py+border_thickness,ydim-border_thickness);

      for (unsigned int qy=qy_lo; qy <= qy_hi; qy++)
      {
         for (unsigned int qx=qx_lo; qx <= qx_hi; qx++)
         {
            if (nearly_equal(cc_twoDarray_ptr->get(qx,qy),null_value))
            {
               cc_twoDarray_ptr->put(qx,qy,border_value);
            }
         } // loop over qx index
      } // loop over qy index

   } // loop over index i labeling all pixels within extremal region

   border_pixels.clear();
   for (unsigned int py=0; py<ydim; py++)
   {
      for (unsigned int px=0; px<xdim; px++)
      {
         if (nearly_equal(cc_twoDarray_ptr->get(px,py),border_value))
         {
            pair<int,int> P(px,py);
            border_pixels.push_back(P);
         }
      } // loop over px 
   } // loop over py

//   cout << "border_pixels.size() = " << border_pixels.size() << endl;
   
   return border_pixels;
}

// =========================================================================
// Adjacent extremal region member functions
// =========================================================================

// Member function add_adjacent_region()

void extremal_region::add_adjacent_region(extremal_region* extremal_region_ptr)
{
//   cout << "inside extremal_region::add_adjacent_region()" << endl;

   int extremal_region_ID=extremal_region_ptr->get_ID();
   adjacent_regions_iter=adjacent_regions_map.find(extremal_region_ID);
   if (adjacent_regions_iter != adjacent_regions_map.end())
   {
      adjacent_regions_map[extremal_region_ID]=extremal_region_ptr;
   }
}

// ---------------------------------------------------------------------
// Member function delete_adjacent_region()

void extremal_region::delete_adjacent_region(int region_ID)
{
//   cout << "inside extremal_region::delete_adjacent_region()" << endl;

   adjacent_regions_iter=adjacent_regions_map.find(region_ID);
   if (adjacent_regions_iter != adjacent_regions_map.end())
   {
      adjacent_regions_map.erase(adjacent_regions_iter);
   }
}
