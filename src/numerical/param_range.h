// ==========================================================================
// Header file for param_range class 
// ==========================================================================
// Last modified on 8/31/05; 7/16/05; 10/11/08; 11/16/09
// ==========================================================================

#ifndef PARAM_RANGE_H
#define PARAM_RANGE_H

#include <vector>

class param_range
{

  public:

// Initialization, constructor and destructor functions:

   param_range(int Nbins,bool ispf=false);
   param_range(double Start,double Stop,int Nbins,bool ispf=false);
   param_range(double Start,double Stop,double Delta,bool ispf=false);
   ~param_range();

//   param_range& operator= (const param_range& p);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const param_range& p);

// Set & get member functions:

   void set_include_stop_point_flag(bool ispf);
   void set_start(double start);
   double get_start() const;
   void set_stop(double stop);
   double get_stop() const;
   double get_delta() const;
   int get_nbins() const;
   int get_counter() const;
   double get_value() const;
   double get_next_value() const;
   double get_avg_value() const;

   void set_best_value();
   void set_best_value(double v);
   double get_best_value() const;

   double compute_value(int n) const;
   bool prepare_next_value();
   std::vector<double>* get_value_sequence();
   void shrink_search_interval(double center_value,double frac=0.1);
   void shrink_search_interval(
      double center_value,double hard_min_value,double hard_max_value,
      double frac);
   void reset_nbins(int n);
   void decrement_nbins();

  private: 

   bool include_stop_point_flag;
   int counter;
   int nbins;
   double start,stop,delta,value,best_value;

   void allocate_member_objects();
   void initialize_member_objects();

   int get_final_counter_index() const;

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void param_range::set_include_stop_point_flag(bool ispf)
{
   include_stop_point_flag=ispf;
}

inline void param_range::set_start(double start)
{
   this->start=start;
}

inline double param_range::get_start() const
{
   return start;
}

inline void param_range::set_stop(double stop)
{
   this->stop=stop;
}

inline double param_range::get_stop() const
{
   return stop;
}

inline double param_range::get_delta() const
{
   return delta;
}

inline int param_range::get_nbins() const
{
   return nbins;
}

inline int param_range::get_counter() const
{
   return counter;
}

inline double param_range::get_value() const
{
   return value;
}

inline double param_range::get_next_value() const
{
   return value+delta;
}

inline void param_range::set_best_value()
{
   best_value=value;
}

inline void param_range::set_best_value(double v)
{
   best_value=v;
}

inline double param_range::get_best_value() const
{
   return best_value;
}


#endif  // numerical/param_range.h





