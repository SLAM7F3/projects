// ==========================================================================
// Datapoint class member function definitions
// ==========================================================================
// Last modified on 1/12/04; 12/4/10
// ==========================================================================

#include "math/basic_math.h"
#include "datastructures/datapoint.h"
#include "templates/mytemplates.h"

// ==========================================================================
// Initialization, constructor and destructor methods:
// ==========================================================================

void datapoint::allocate_member_objects() 
{
   new_clear_array(var,n_indep_vars);

// CLUGE inserted on 1/12/04 in order to avoid memory problems within
// mypolynomial::fit_coeffs_using_residuals().  Someday we'll clean
// this up.

   new_clear_array(func,basic_math::max(2,n_depend_vars));
}

void datapoint::initialize_member_objects()
{
   point_size=1;
   color=colorfunc::blue;
}

datapoint::datapoint()
{
}

datapoint::datapoint(
   const double variable,const double function_value)
{
   n_indep_vars=1;
   n_depend_vars=1;
   allocate_member_objects();
   initialize_member_objects();
   var[0]=variable;
   func[0]=function_value;
}

datapoint::datapoint(
   const double variable,const double function_value,const double error)
{
   n_indep_vars=1;
   n_depend_vars=2;
   allocate_member_objects();
   initialize_member_objects();
   var[0]=variable;
   func[0]=function_value;
   func[1]=error;
}

datapoint::datapoint(
   const int n_indep_variables,const double variable[],
   const double function_value)
{
   n_indep_vars=n_indep_variables;
   n_depend_vars=1;
   allocate_member_objects();
   initialize_member_objects();
   for (int n=0; n<n_indep_vars; n++)
   {
      var[n]=variable[n];
   }
   func[0]=function_value;
}

datapoint::datapoint(
   const int n_depend_variables,const double variable,
   const double function_value[])
{
   n_indep_vars=1;
   n_depend_vars=n_depend_variables;
   allocate_member_objects();
   initialize_member_objects();
   var[0]=variable;
   for (int n=0; n<n_depend_vars; n++)
   {
      func[n]=function_value[n];
   }
}

datapoint::datapoint(
   const int n_indep_variables,const int n_depend_variables,
   const double variable[],const double function_value[])
{
   n_indep_vars=n_indep_variables;
   n_depend_vars=n_depend_variables;
   allocate_member_objects();
   initialize_member_objects();
   for (int n=0; n<n_indep_vars; n++)
   {
      var[n]=variable[n];
   }
   for (int n=0; n<n_depend_vars; n++)
   {
      func[n]=function_value[n];
   }
}

// Copy constructor:

datapoint::datapoint(const datapoint& n)
{
   docopy(n);
}

datapoint::~datapoint()
{
   delete [] var;
   delete [] func;
   var=NULL;
   func=NULL;
}

// ---------------------------------------------------------------------
void datapoint::docopy(const datapoint& n)
{
   n_indep_vars=n.n_indep_vars;
   n_depend_vars=n.n_depend_vars;
   allocate_member_objects();
   point_size=n.point_size;
   color=n.color;

   for (int i=0; i<n_indep_vars; i++)
   {
      var[i]=n.var[i];
   }
   for (int i=0; i<n_depend_vars; i++)
   {
      func[i]=n.func[i];
   }
}

// Overload = operator:

datapoint& datapoint::operator= (const datapoint& n)
{
   if (this==&n) return *this;
   docopy(n);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

std::ostream& operator<< (std::ostream& outstream,const datapoint& n)
{
   outstream << std::endl;
   outstream << "n_indep_vars = " << n.n_indep_vars << std::endl;
   for (int i=0; i<n.n_indep_vars; i++)
   {
      outstream << "i = " << i << " var[i] = " << n.var[i] << std::endl;
   }
   for (int i=0; i<n.n_depend_vars; i++)
   {
      outstream << "i = " << i << " func[i] = " << n.func[i] << std::endl;
   }
   return outstream;
}








