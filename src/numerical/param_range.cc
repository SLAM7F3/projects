// ==========================================================================
// Param_Range class member function definitions
// ==========================================================================
// Last modified on 10/11/08; 11/16/09; 12/4/10
// ==========================================================================

#include <iostream>
#include "math/basic_math.h"
#include "numerical/param_range.h"

using std::cout;
using std::endl;
using std::ostream;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void param_range::allocate_member_objects()
{
}		       

void param_range::initialize_member_objects()
{
   counter=0;
}

param_range::param_range(int Nbins,bool ispf)
{
   allocate_member_objects();
   initialize_member_objects();
   nbins=Nbins;
   include_stop_point_flag=ispf;
   if (nbins <= 0)
   {
      cout << "Error in param_range constructor, nbins = " << nbins 
           << endl;
      exit(-1);
   }
   else if (nbins==1)
   {
      delta=0;
   }
}

param_range::param_range(double Start,double Stop,int Nbins,bool ispf)
{
   allocate_member_objects();
   initialize_member_objects();
   start=Start;
   stop=Stop;
//   nbins=Nbins;
   include_stop_point_flag=ispf;

   reset_nbins(Nbins);
/*
   if (nbins <= 0)
   {
      cout << "Error in param_range constructor, nbins = " << nbins 
           << endl;
      exit(-1);
   }
   else if (nbins==1)
   {
      delta=0;
   }
   else
   {
      delta=(stop-start)/(nbins-1);
   }
*/

}

param_range::param_range(double Start,double Stop,double Delta, bool ispf)
{
   allocate_member_objects();
   initialize_member_objects();
   start=Start;
   stop=Stop;
   delta=Delta;
   include_stop_point_flag=ispf;

   if (delta < 0)
   {
      cout << "Error in param_range constructor, delta = " << delta
           << endl;
      exit(-1);
   }
   else if (nearly_equal(delta,0))
   {
      nbins=1;
   }
   else
   {
      nbins=(stop-start)/delta+1;

// Revise stop member so that it corresponds to a perfect integer
// number of delta spacings from start member:

      stop=start+get_final_counter_index()*delta;
   }
}

param_range::~param_range()
{
}

// ---------------------------------------------------------------------   
// Overload << operator

ostream& operator<< (ostream& outstream,const param_range& p)
{
   outstream.precision(10);
   outstream << "nbins = " << p.nbins << endl;
   outstream << "start = " << p.start << " stop = " << p.stop 
             << " delta = " << p.delta << endl;
   outstream << "average value = " << p.get_avg_value() << endl;
   return(outstream);
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

// Member function reset_nbins

void param_range::reset_nbins(int n)
{
   nbins=n;

   if (nbins <= 0)
   {
      cout << "Error in param_range::reset_nbins(), nbins = " << nbins 
           << endl;
      exit(-1);
   }
   else if (nbins==1)
   {
      delta=0;
   }
   else
   {
      delta=(stop-start)/(nbins-1);
   }
}

void param_range::decrement_nbins()
{
   reset_nbins(nbins-1);
}

// ==========================================================================
double param_range::compute_value(int n) const
{
   return start+n*delta;
/*
   if (n < 0 || n >= nbins)
   {
      cout << "Error in param_range::compute_value()" << endl;
      cout <<  "n = " << n << " lies outside valid index range" << endl;
   }
   else
   {
      return start+n*delta;
   }
*/
}

// ---------------------------------------------------------------------   
int param_range::get_final_counter_index() const
{
   if (include_stop_point_flag)
   {
      return nbins+1;
   }
   else
   {
      return nbins;
   }
}

// ---------------------------------------------------------------------   
bool param_range::prepare_next_value()
{
   if (counter < get_final_counter_index())
   {
      value=start+counter*delta;
      counter++;
      return true;
   }
   else
   {
      counter=0;
      return false;
   }
}

// ---------------------------------------------------------------------
// Member function get_avg_value returns the average of the first and
// last bins' values.

double param_range::get_avg_value() const
{
   return 0.5*(compute_value(0)+compute_value(get_final_counter_index()));
}

// ---------------------------------------------------------------------
// Member function get_value_sequence dynamically generates, fills and
// returns an STL vector with nbins parameter values ranging between
// start and stop.

vector<double>* param_range::get_value_sequence()
{
   vector<double>* param_value_ptr=new vector<double>;
   for (int n=0; n<get_final_counter_index(); n++)
   {
      param_value_ptr->push_back(start+n*delta);
   }
   return param_value_ptr;
}

// ---------------------------------------------------------------------
// Member function shrink_search_interval takes in parameter
// center_value about which a new search interval is to be formed as
// well as fraction frac of the current interval's length.  It resets
// member variables start, stop and delta, but leaves nbins unchanged.
// This method should help with iteratively performing searches over
// parameter spaces where we want to home in on the location of some
// function's extremal value.

void param_range::shrink_search_interval(double center_value,double frac)
{
   if (nbins > 1)
   {
      double interval=frac*(stop-start);
      start=center_value-0.5*interval;
      stop=center_value+0.5*interval;
      delta=(stop-start)/(nbins-1);   
   }
   counter=0;
}

// This overloaded version of shrink_search_interval() takes in hard
// min/max values for the search parameter.  The new search interval
// is guaranteed to lie within the interval
// [hard_min_value,hard_max_value].

void param_range::shrink_search_interval(
   double center_value,double hard_min_value,double hard_max_value,
   double frac)
{
   if (nbins > 1)
   {
      double interval=frac*(stop-start);
      start=center_value-0.5*interval;
      stop=center_value+0.5*interval;
      start=basic_math::max(hard_min_value,start);
      stop=basic_math::min(hard_max_value,stop);
      delta=(stop-start)/(nbins-1);   
   }
   counter=0;
}

