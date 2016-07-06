// ==========================================================================
// Header file for datapoint class
// ==========================================================================
// Last modified on 1/12/04; 1/14/10
// ==========================================================================

#ifndef DATAPOINT_H
#define DATAPOINT_H

#include "color/colorfuncs.h"

class datapoint
{

  public:

// Initialization, constructor and destructor functions:

   datapoint();
   datapoint(const double variable,const double function_value);
   datapoint(const double variable,const double function_value,
             const double error);
   datapoint(const int n_indep_variables,const double variable[],
             double function_value);
   datapoint(const int n_depend_variables,double variable,
             const double function_value[]);
   datapoint(const int n_indep_variables,const int n_depend_variables,
             const double variable[],const double function_value[]);
   datapoint(const datapoint& n);
   virtual ~datapoint();

   datapoint& operator= (const datapoint& n);

   friend std::ostream& operator<< 
      (std::ostream& outstream,const datapoint& n);

// Set and get member functions:
   
   void set_n_depend_vars(int n);
   void set_point_size(double size);
   void set_color(const colorfunc::Color c);
   void set_var(int n,double d);
   void set_func(double* func_ptr);
   void set_func(int n,double d);
   int get_n_indep_vars() const;
   int get_n_depend_vars() const;
   double get_point_size() const;
   colorfunc::Color get_color() const;
   double get_var(int n) const;
   double* get_func();
   double get_func(int n) const;

  private:

   int n_indep_vars,n_depend_vars;

// Metafile parameters:

   double point_size;
   colorfunc::Color color;

   double *var;		// independent variables
   double *func;        // dependent objects

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const datapoint& n);

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:

inline void datapoint::set_n_depend_vars(int n)
{
   n_depend_vars=n;
}

inline void datapoint::set_point_size(double size)
{
   point_size=size;
}

inline void datapoint::set_color(const colorfunc::Color c)
{
   color=c;
}

inline void datapoint::set_var(int n,double d)
{
   var[n]=d;
}

inline void datapoint::set_func(double* func_ptr)
{
   func=func_ptr;
}

inline void datapoint::set_func(int n,double d)
{
   func[n]=d;
}

inline int datapoint::get_n_indep_vars() const
{
   return n_indep_vars;
}

inline int datapoint::get_n_depend_vars() const
{
   return n_depend_vars;
}

inline double datapoint::get_point_size() const
{
   return point_size;
}

inline colorfunc::Color datapoint::get_color() const
{
   return color;
}

inline double datapoint::get_var(int n) const
{
   return var[n];
}

inline double* datapoint::get_func() 
{
   return func;
}

inline double datapoint::get_func(int n) const
{
   return func[n];
}

#endif  // datastructures/datapoint.h



