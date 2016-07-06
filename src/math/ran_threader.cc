// =========================================================================
// Ran_Threader class member function definitions
// =========================================================================
// Last modified on 7/4/13
// =========================================================================

#include <iostream>
#include <math.h>
#include <string>
#include <vector>

#include "math/basic_math.h"
#include "templates/mytemplates.h"
#include "math/ran_threader.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ofstream;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void ran_threader::allocate_member_objects()
{
}

void ran_threader::initialize_member_objects()
{
}		 

// ---------------------------------------------------------------------
ran_threader::ran_threader()
{
   initialize_member_objects();
   allocate_member_objects();
}

// ---------------------------------------------------------------------
// Copy constructor:

ran_threader::ran_threader(const ran_threader& rt)
{
//   cout << "inside ran_threader copy constructor, this(ran_threader) = " << this << endl;
   initialize_member_objects();
   allocate_member_objects();
   docopy(rt);
}

ran_threader::~ran_threader()
{
//   cout << "inside ran_threader destructor" << endl;

   for (iter=random_map.begin(); iter != random_map.end(); iter++)
   {
      delete iter->second.first;
   }
}

// ---------------------------------------------------------------------
void ran_threader::docopy(const ran_threader& rt)
{
//   cout << "inside ran_threader::docopy()" << endl;
//   cout << "this = " << this << endl;
}

// Overload = operator:

ran_threader& ran_threader::operator= (const ran_threader& rt)
{
//   cout << "inside ran_threader::operator=" << endl;
//   cout << "this(ran_threader) = " << this << endl;
   if (this==&rt) return *this;
   docopy(rt);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const ran_threader& rt)
{
   outstream << endl;
//   outstream << "Ran_Threader ID = " << e.ID << endl;
   
   return outstream;
}

// =========================================================================
// Member function create_rand_object(int thread_i) instantiates a
// dlib::rand() object corresponding to the input non-negative integer
// index thread_i which we assume is unique:

void ran_threader::create_rand_object(int thread_i)
{
   create_new_dlib_rand_object(thread_i);
}

// Member function create_rand_object(int thread_i,int thread_j)
// instantiates a dlib::rand() object corresponding to input
// non-negative integer indices thread_i and thread_j which we assume
// are unique:

void ran_threader::create_rand_object(int thread_i,int thread_j)
{
   create_new_dlib_rand_object(thread_i+1E-8*thread_j);
}

void ran_threader::create_new_dlib_rand_object(double curr_x)
{
   dlib::rand* rnd_ptr=new dlib::rand();
   int seed_call_counter=0;
   pair<dlib::rand*,int> P(rnd_ptr,seed_call_counter);
   random_map[curr_x]=P;
}

double ran_threader::_get_random_double(double curr_x)
{

// Davis says that this is NOT thread safe!  Need to mutex lock 

   iter=random_map.find(curr_x);
   if (iter==random_map.end())
   {
      create_new_dlib_rand_object(curr_x);
      iter=random_map.find(curr_x);
   }

   iter->second.second++;
   dlib::rand* rnd_ptr=iter->second.first;

/*
   int seed_counter=iter->second.second;
   double random_var=rnd_ptr->get_random_double();
   cout << "curr_x = " << curr_x
        << " counter = " << seed_counter 
        << " random_var = " << random_var << endl;
   return random_var;
*/
   return rnd_ptr->get_random_double();
}


// =========================================================================
// Random sequence member functions
// =========================================================================

vector<int> ran_threader::random_sequence(int nsize)
{
   return threaded_random_sequence(0,nsize-1,nsize,0,0);
}

vector<int> ran_threader::random_sequence(
   int nsize,int sequence_length)
{
   return threaded_random_sequence(0,nsize-1,sequence_length,0,0);
}

vector<int> ran_threader::random_sequence(
   int nstart,int nstop,int sequence_length)
{
   return threaded_random_sequence(
      nstart,nstop,sequence_length,0,0);
}



vector<int> ran_threader::random_sequence_threaded(int nsize,int thread_i)
{
   return threaded_random_sequence(0,nsize-1,nsize,thread_i,0);
}

vector<int> ran_threader::random_sequence_threaded(
   int nsize,int sequence_length,int thread_i)
{
   return threaded_random_sequence(
      0,nsize-1,sequence_length,thread_i,0);
}

vector<int> ran_threader::random_sequence_threaded(
   int nstart,int nstop,int sequence_length,int thread_i)
{
   return threaded_random_sequence(
      nstart,nstop,sequence_length,thread_i,0);
}


vector<int> ran_threader::threaded_random_sequence(
   int nsize,int thread_i,int thread_j)
{
   return threaded_random_sequence(0,nsize-1,nsize,thread_i,thread_j);
}

vector<int> ran_threader::threaded_random_sequence(
   int nsize,int sequence_length,int thread_i,int thread_j)
{
   return threaded_random_sequence(
      0,nsize-1,sequence_length,thread_i,thread_j);
}

// ---------------------------------------------------------------------
// This final version of threaded_random_sequence() returns the first
// sequence_length entries within a randomized sequence of [istart ,
// istart+1 , ... , istop].

vector<int> ran_threader::threaded_random_sequence(
   int nstart,int nstop,int sequence_length,int thread_i,int thread_j)
{
//      cout << "inside ran_threader::threaded_random_sequence()" << endl;

   int nsize=nstop-nstart+1;
   vector<int> a,a_random;
   a.reserve(nsize);

   for (int n=0; n<nsize; n++) 
   {
      a.push_back(nstart+n);
   }

// Durstenfeld's random number shuffling algorithm: Move "struck"
// numbers to end of list by swapping them with last unstruck number at
// each interation.  Time complexity of this algorithm is O(N) rather
// than O(N**2).  See http://en.wikipedia.org/wiki/Fisher-Yates_shuffle.

   for (int n=nsize-1; n>=nsize-sequence_length; n--)
   {
      int m=basic_math::round(n*get_random_double(thread_i,thread_j)); 
      a_random.push_back(a[m]);
      templatefunc::swap(a[m],a[n]);
   }

   return a_random;
}

