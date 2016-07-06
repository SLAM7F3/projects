// ==========================================================================
// Header file for ran_threader class
// ==========================================================================
// Last modified on 7/4/13
// ==========================================================================

#ifndef RAN_THREADER_H
#define RAN_THREADER_H

#include <algorithm>
#include <map>
#include <vector>
#include <dlib/rand.h>

class ran_threader
{

  public:

   typedef std::map<double,std::pair<dlib::rand*,int> > RANDOM_MAP;

// independent double var = 
//	one input integer i   OR   
//	two input integers i & j chained together into decimal number 

// dependent var = pair containing 
//    1. pointer to dynamically instantiated dlib::rand() object 
//    2. number of calls made to this particular dlib::rand() object

   ran_threader();
   ran_threader(const ran_threader& rt);
   ~ran_threader();
   ran_threader& operator= (const ran_threader& rt);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const ran_threader& rt);

// Set and get member functions:

   double get_random_double();
   double get_random_double(int thread_i);
   double get_random_double(int thread_i,int thread_j);

   void create_rand_object(int thread_i);
   void create_rand_object(int thread_i,int thread_j);
   double _get_random_double(double x);

// Random sequence member functions:

   std::vector<int> random_sequence(int nize);
   std::vector<int> random_sequence(int nize,int sequence_length);
   std::vector<int> random_sequence(
      int nstart,int nstop,int sequence_length);

   std::vector<int> random_sequence_threaded(
      int nsize,int thread_i);
   std::vector<int> random_sequence_threaded(
      int nsize,int sequence_length,int thread_i);
   std::vector<int> random_sequence_threaded(
      int nstart,int nstop,int sequence_length,int thread_i);

   std::vector<int> threaded_random_sequence(
      int nsize,int thread_i,int thread_j);
   std::vector<int> threaded_random_sequence(
      int nsize,int sequence_length,int thread_i,int thread_j);
   std::vector<int> threaded_random_sequence(
      int nstart,int nstop,int sequence_length,
      int thread_i,int thread_j);

  private: 

   RANDOM_MAP random_map;
   RANDOM_MAP::iterator iter;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const ran_threader& rt);

   void create_new_dlib_rand_object(double curr_x);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline double ran_threader::get_random_double()
{
   return _get_random_double(0);
}

inline double ran_threader::get_random_double(int thread_i)
{
   return _get_random_double(thread_i);
}

inline double ran_threader::get_random_double(int thread_i,int thread_j)
{
   return _get_random_double(thread_i+1E-8*thread_j);
}


#endif  // ran_threader.h
