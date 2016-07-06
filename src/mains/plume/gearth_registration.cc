// ==========================================================================
// Program GEARTH_REGISTRATION

//				gearth_registration


// ==========================================================================
// Last updated on 1/3/13; 1/4/13; 1/7/13; 11/21/13
// ==========================================================================

#include  <iostream>
#include  <string>
#include  <vector>

#include "geometry/affine_transform.h"
#include "math/threevector.h"
#include "math/twovector.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
   cout.precision(12);

   vector<twovector> p_vecs,q_vecs;

// p = A q + trans

// sc_site_5 UV coords --> sc_site_4 UV coords  

   q_vecs.push_back(twovector(0.7366462946,  0.5260299444));
   q_vecs.push_back(twovector(0.8481956720,  0.5562225580));
   q_vecs.push_back(twovector(0.7774845362,  0.3627600074));
   q_vecs.push_back(twovector(0.9926769733,  0.3786882162));
   q_vecs.push_back(twovector(0.6864808798,  0.4787641764));
   q_vecs.push_back(twovector(0.8186390400,  0.4810260236));
   q_vecs.push_back(twovector(1.012278795,  0.4931911230));
   q_vecs.push_back(twovector(0.7328299880,  0.4102273285));
   q_vecs.push_back(twovector(0.8737969398,  0.4703331590));
   q_vecs.push_back(twovector(0.9553337097,  0.4659391642));

   p_vecs.push_back(twovector(0.2883555591,  0.8421756029));
   p_vecs.push_back(twovector(0.7816376686,  0.9739847779));
   p_vecs.push_back(twovector(0.4448386431,  0.1091903597));
   p_vecs.push_back(twovector(1.409126520,  0.1720196456));
   p_vecs.push_back(twovector(0.06095719337,  0.6450220346));
   p_vecs.push_back(twovector(0.6429300308,  0.6453862190));
   p_vecs.push_back(twovector(1.503875732,  0.6826280355));
   p_vecs.push_back(twovector(0.2522548437,  0.3313505650));
   p_vecs.push_back(twovector(0.8887836337,  0.5917636156));
   p_vecs.push_back(twovector(1.250389338,  0.5654659271));

/*

// sc_site_4 UV coords --> surveyed truth coords   

// UV image plane feature locations in SC_site_4.png.  We
// intentionally tried to chose UV points located near base of wooden
// posts to minimize perspective problems:

   q_vecs.push_back(twovector(0.8880232573,0.5851291418));	// NE
   q_vecs.push_back(twovector(0.8622721434,0.3259973824));	// SE
   q_vecs.push_back(twovector(0.6006399393,0.3576030135));	// SW
   q_vecs.push_back(twovector(0.6438599825,0.6376193762));	// NW

//   q_vecs.push_back(twovector(0.8893847466,0.5902867913)); 	// NE  
//   q_vecs.push_back(twovector(0.8631272316,0.3356850147)); 	// SE
//   q_vecs.push_back(twovector(0.6019337177,0.3675101995));   	// SW
//   q_vecs.push_back(twovector(0.6420110464,0.6428675652));   	// NW

// Fitted surveyed locations for middle wooden posts:

   p_vecs.push_back(twovector(25.39397778,23.99390857));	// NE
   p_vecs.push_back(twovector(24.70671364,-21.60018909));	// SE
   p_vecs.push_back(twovector(-23.54938968,-24.71375663));	// SW
   p_vecs.push_back(twovector(-23.8489854,30.19430996));	// NW
*/

//   twovector trans;
//   genmatrix A(2,2);
//   double S=mathfunc::fit_2D_affine_transformation(q_vecs,p_vecs,A,trans);

   affine_transform* affine_transform_ptr=new affine_transform();
   affine_transform_ptr->parse_affine_transform_inputs(q_vecs,p_vecs);
   affine_transform_ptr->fit_affine_transformation();
   double RMS_residual=affine_transform_ptr->check_affine_transformation(
      q_vecs,p_vecs);

   genmatrix A(2,2);
   A=*(affine_transform_ptr->get_A_ptr());
   twovector trans=affine_transform_ptr->get_trans();


   cout << "A = " << A << endl;
   cout << "trans = " << trans << endl;
   cout << "RMS_residual = " << RMS_residual << endl;

   double RMS=0;
   for (int i=0; i<q_vecs.size(); i++)
   {
      twovector p_calc=A*q_vecs[i]+trans;
      double sqr_delta=(p_calc-p_vecs[i]).sqrd_magnitude();
      RMS += sqr_delta;
   }
   RMS /= q_vecs.size();
   RMS=sqrt(RMS);
   cout << "RMS error = " << RMS << endl;

   double Umin=0;
//   double Umax=1983.0/1299.0;		// sc_site_4.png
   double Umax=1873.0/1306.0;		// sc_site_5.png
   double Vmin=0;
   double Vmax=1;

   vector<twovector> corner_vecs;
   corner_vecs.push_back(twovector(Umax,Vmax));
   corner_vecs.push_back(twovector(Umax,Vmin));
   corner_vecs.push_back(twovector(Umin,Vmin));
   corner_vecs.push_back(twovector(Umin,Vmax));

   cout << "Umax = " << Umax << endl;

// Convert from sc_site4 coordinates to survey coordinates:

   genmatrix B(2,2);
   B.put(0,0,188.606881417);
   B.put(0,1,-23.7121404348);
   B.put(1,0,24.7176425265);
   B.put(1,1,183.651570659);
   
   twovector trans2;
   trans2.put(0,-129.233269188);
   trans2.put(1,-104.063494918);

   vector<twovector> survey_vecs;
   for (int i=0; i<q_vecs.size(); i++)
   {
      twovector p_calc=A*q_vecs[i]+trans;
      survey_vecs.push_back(B*p_calc+trans2);

      cout << "i = " << i 
           << " sc_site_5: U = " << q_vecs[i].get(0)
           << " V = " << q_vecs[i].get(1) 
           << " survey: X = " << survey_vecs.back().get(0) 
           << " Y = " << survey_vecs.back().get(1)
           << endl;
   }

// Add tree features to sc_site_5 q_vecs and survey_vecs:

   q_vecs.push_back(twovector(0.9207242727,0.6823477745));		// B
   q_vecs.push_back(twovector(0.8953246474,0.6869026423));		// C
   q_vecs.push_back(twovector(0.8790234327,0.6888005137));		// D
   q_vecs.push_back(twovector(0.8505197763,0.6901290417));		// E
   q_vecs.push_back(twovector(0.8370617628,0.6941145658));		// F
   q_vecs.push_back(twovector(0.8035820723,0.6977205276));		// J
   q_vecs.push_back(twovector(0.7914509177,0.6994285583));		// K
   q_vecs.push_back(twovector(0.7503435612,0.6957933903));		// M
   
   survey_vecs.push_back(twovector(59.29980850,282.9490967));		// B
   survey_vecs.push_back(twovector(28.06726646,284.4897461));		// C
   survey_vecs.push_back(twovector(9.288945198,284.1076660));		// D
   survey_vecs.push_back(twovector(-18.86486244,282.1926270));		// E
   survey_vecs.push_back(twovector(-41.49486923,288.1386719));		// F
   survey_vecs.push_back(twovector(-78.49291229,291.9199219));		// J
   survey_vecs.push_back(twovector(-93.96508026,294.4227295));		// K
   survey_vecs.push_back(twovector(-147.6195068,286.5604248));		// M

   cout << "q_vecs.size() = " << q_vecs.size()
        << " survey_vecs.size() = " << survey_vecs.size() << endl;

   affine_transform_ptr->parse_affine_transform_inputs(q_vecs,survey_vecs);
   affine_transform_ptr->fit_affine_transformation();
   RMS_residual=affine_transform_ptr->check_affine_transformation(
      q_vecs,survey_vecs);

   genmatrix D(2,2);   
   D=*(affine_transform_ptr->get_A_ptr());
   twovector trans3=affine_transform_ptr->get_trans();

   delete affine_transform_ptr;

// survey_vec = D * q_vec + trans3
   
   cout << "D = " << D << endl;
   cout << "trans3 = " << trans3 << endl;

   for (int i=0; i<corner_vecs.size(); i++)
   {
      twovector corner_survey=D*corner_vecs[i]+trans3;

      cout << "i = " << i 
           << " corner survey: X = " << corner_survey.get(0)
           << " Y = " << corner_survey.get(1)
           << endl;
   }


}

