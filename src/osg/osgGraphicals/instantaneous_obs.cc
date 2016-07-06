// ==========================================================================
// INSTANTANEOUS_OBS class member function definitions
// ==========================================================================
// Last modified on 5/26/09; 5/25/10; 5/31/10; 1/21/13; 4/6/14
// ==========================================================================

#include <iostream>
#include "osg/osgGraphicals/instantaneous_obs.h"

using std::cout;
using std::endl;
using std::map;
using std::ostream;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void instantaneous_obs::allocate_member_objects()
{
   multiple_image_coords_ptr=new THREEVECTOR_MAP;
   multiple_image_vertices_vec_ptr=new VECTHREEVECTOR_VEC;
   multiple_image_attitude_ptr=new FOURVECTOR_MAP;
   multiple_image_scale_ptr=new THREEVECTOR_MAP;
   multiple_image_velocity_ptr=new THREEVECTOR_MAP;
   multiple_image_score_ptr=new DOUBLE_MAP;   
   multiple_image_index_ptr=new INT_MAP;   
}		       

void instantaneous_obs::initialize_member_objects()
{
   t=0;	// secs
   U_scale=V_scale=W_scale=1.0;
   U_hat=threevector(1,0,0);
   V_hat=threevector(0,1,0);
   W_hat=threevector(0,0,1);
}		       

instantaneous_obs::instantaneous_obs()
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

instantaneous_obs::instantaneous_obs(double curr_t)
{	
   initialize_member_objects();
   allocate_member_objects();
   set_t(curr_t);
}		       

// ---------------------------------------------------------------------
// Copy constructor:

instantaneous_obs::instantaneous_obs(const instantaneous_obs& io)
{
//   cout << "inside instantaneous_obs copy constructor()" << endl;
   initialize_member_objects();
   allocate_member_objects();
   docopy(io);
}

instantaneous_obs::~instantaneous_obs()
{
   delete multiple_image_coords_ptr;
   delete multiple_image_vertices_vec_ptr;
   delete multiple_image_attitude_ptr;
   delete multiple_image_scale_ptr;
   delete multiple_image_velocity_ptr;
   delete multiple_image_score_ptr;
   delete multiple_image_index_ptr;
}

// ---------------------------------------------------------------------
void instantaneous_obs::docopy(const instantaneous_obs& io)
{
   multiple_image_coords_ptr->clear();
   multiple_image_vertices_vec_ptr->clear();
   multiple_image_attitude_ptr->clear();
   multiple_image_scale_ptr->clear();
   multiple_image_velocity_ptr->clear();
   multiple_image_score_ptr->clear();
   multiple_image_index_ptr->clear();

   for (THREEVECTOR_MAP::iterator iter=io.multiple_image_coords_ptr->begin();
        iter != io.multiple_image_coords_ptr->end(); iter++)
   {
      (*multiple_image_coords_ptr)[iter->first]=iter->second;
   }

   for (unsigned int i=0; i<io.multiple_image_vertices_vec_ptr->size(); i++)
   {
      multiple_image_vertices_vec_ptr->push_back(
         io.multiple_image_vertices_vec_ptr->at(i));
   }

   for (FOURVECTOR_MAP::iterator iter=io.multiple_image_attitude_ptr->
           begin(); iter != io.multiple_image_attitude_ptr->end(); iter++)
   {
      (*multiple_image_attitude_ptr)[iter->first]=iter->second;
   }

   for (THREEVECTOR_MAP::iterator iter=io.multiple_image_scale_ptr->
           begin(); iter != io.multiple_image_scale_ptr->end(); iter++)
   {
      (*multiple_image_scale_ptr)[iter->first]=iter->second;
   }

   for (THREEVECTOR_MAP::iterator iter=io.multiple_image_velocity_ptr->
           begin(); iter != io.multiple_image_velocity_ptr->end(); iter++)
   {
      (*multiple_image_velocity_ptr)[iter->first]=iter->second;
   }

   for (DOUBLE_MAP::iterator iter=io.multiple_image_score_ptr->
           begin(); iter != io.multiple_image_score_ptr->end(); iter++)
   {
      (*multiple_image_score_ptr)[iter->first]=iter->second;
   }

   for (INT_MAP::iterator iter=io.multiple_image_index_ptr->
           begin(); iter != io.multiple_image_index_ptr->end(); iter++)
   {
      (*multiple_image_index_ptr)[iter->first]=iter->second;
   }

   t=io.t;
   U_scale=io.U_scale;
   V_scale=io.V_scale;
   W_scale=io.W_scale;
   U_hat=io.U_hat;
   V_hat=io.V_hat;
   W_hat=io.W_hat;
}

// Overload = operator:

instantaneous_obs& instantaneous_obs::operator= (const instantaneous_obs& i)
{
   if (this==&i) return *this;
   docopy(i);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const instantaneous_obs& obs)
{
   outstream << "inside instantaneous_obs::operator<<" << endl;
   outstream << "Time t = " << obs.t << endl;

   outstream << "multiple_image_coords_ptr->size() = "
             << obs.multiple_image_coords_ptr->size() << endl;

   for (instantaneous_obs::THREEVECTOR_MAP::iterator iter=
           obs.multiple_image_coords_ptr->begin();
        iter != obs.multiple_image_coords_ptr->end(); iter++)
   {
      outstream << "pass = " << iter->first
                << " coords = " << iter->second
                << endl;
   }

   outstream << "At end of instantaneous_obs::operator<< " << endl;

   return outstream;
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

// Member function get_pass_numbers() fills and returns a STL member
// vector pass_numbers which contains integer pass numbers for the
// current instantaneous observation.

vector<int>& instantaneous_obs::get_pass_numbers() 
{
//   cout << "inside instantaneous_obs::get_pass_numbers()" << endl;
   int counter=0;
   pass_numbers.clear();
   for (THREEVECTOR_MAP::iterator iter=multiple_image_coords_ptr->
           begin(); iter != multiple_image_coords_ptr->end(); iter++)
   {
      pass_numbers.push_back(iter->first);
//      cout << "counter = " << counter
//           << " pass_number = " << pass_numbers.back() << endl;
      counter++;
   }
   return pass_numbers;
}

// ==========================================================================
// Pass number manipulation member functions
// ==========================================================================

// Member function find_first_passnumber_greater_than_input() loops
// over the integers within member STL vector pass_numbers.  It
// returns the first integer which exceeds input p.  If no such
// integer is found, this method returns -1.

int instantaneous_obs::find_first_passnumber_greater_than_input(int p) const
{
//   cout << "inside instantaneous_obs::find_first_passnumber_greater_than_input()" << endl;
//   cout << "p = " << p 
//        << " pass_numbers.size() = " << pass_numbers.size() << endl;
   
   for (unsigned int i=0; i<pass_numbers.size(); i++)
   {
//      cout << "i = " << i << " pass_numbers[i] = " << pass_numbers[i]
//           << endl;
      if (pass_numbers[i] > p)
      {
         return pass_numbers[i];
      }
   }
   return -1;
}

// ---------------------------------------------------------------------
// Member function change_passnumber() first checks whether input
// passnumber p_old exists within STL vector member pass_numbers.  If
// so, it copies the UVW coords, vertices, quaternion, scale, score
// and index corresponding to pass p_old onto temporary storage.  This
// method then deletes from member STL maps all traces of the pass
// p_old.  It subsequently inserts the temporary variables into new
// instantaneous observations.  Finally, this method removes p_old
// from pass_numbers and adds p_new.

void instantaneous_obs::change_passnumber(int p_old,int p_new)
{
//   cout << "inside instantaneous_obs::change_passnumber()" << endl;
   
   if (!check_for_pass_entry(p_old)) return;

   threevector p3;
   bool UVW_flag=retrieve_UVW_coords(p_old,p3);
   vector<threevector> vertices;
   bool vertices_flag=retrieve_vertices(p_old,vertices);
   osg::Quat quat;
   bool quat_flag=retrieve_quaternion(p_old,quat);
   threevector scale;
   bool scale_flag=retrieve_scale(p_old,scale);
   threevector velocity;
   bool velocity_flag=retrieve_velocity(p_old,velocity);
   double score=-1;
   bool score_flag=retrieve_score(p_old,score);
   int index=-1;
   bool index_flag=retrieve_index(p_old,index);
   delete_all_coords(p_old);
   
   if (UVW_flag)
   {
      insert_UVW_coords(p_new,p3);
   }
   if (vertices_flag)
   {
      insert_vertices(p_new,vertices);
   }
   if (quat_flag)
   {
      insert_quaternion(p_new,quat);
   }
   if (scale_flag)
   {
      insert_scale(p_new,scale);
   }
   if (velocity_flag)
   {
      insert_velocity(p_new,velocity);
   }
   if (score_flag)
   {
      insert_score(p_new,score);
   }
   if (index_flag)
   {
      insert_index(p_new,index);
   }

// Iterate through STL vector pass_numbers.  Delete p_old from this
// vector:

   for (vector<int>::iterator iter=pass_numbers.begin(); 
        iter != pass_numbers.end(); iter++)
   {
      if (*iter==p_old)
      {
         pass_numbers.erase(iter);
         break;
      }
   }

   pass_numbers.push_back(p_new);
}

// ==========================================================================
// Coordinate insertion member functions
// ==========================================================================

// Member function insert_UVW_coords takes in a pass number along with
// a threevector containing (U,V,W) coordinates for some graphical.
// It stores these threevector posn coordinates in member STL map
// *multiple_image_coords_ptr for later retrieval purposes.

void instantaneous_obs::insert_UVW_coords(
   int pass_number,const threevector& p3)
{
//   cout << "inside instantaneous_obs::insert_UVW_coords()" << endl;
//   cout << "pass_number = " << pass_number << " p3 = " << p3 << endl;
   if (retrieve_multiimage_coords_iter(pass_number)==
       multiple_image_coords_ptr->end())
   {
      (*multiple_image_coords_ptr)[pass_number]=p3;
   }
} 

// ---------------------------------------------------------------------
// Member function insert_vertices takes in a pass number along with
// an STL vector of threevectors containing (U,V,W) coordinates for
// some graphical vertices (e.g. triangle vertex locations).  It
// stores these threevector posn coordinates in member STL map
// *multiple_image_vertices_ptr for later retrieval purposes.

void instantaneous_obs::insert_vertices(
   int pass_number,const vector<threevector>& V)
{
//   cout << "inside instantaneous_obs::insert_vertices()" << endl;
   if (retrieve_multiimage_vertices_iter(pass_number)==
       multiple_image_vertices_vec_ptr->end())
   {
      multiple_image_vertices_vec_ptr->push_back(V);
   }
} 

// ---------------------------------------------------------------------
// Member function insert_quaternion takes in a pass number along with
// a quaternion for some graphical.  It stores the quaternion in
// member STL map *multiple_image_attitude_ptr for later retrieval
// purposes.

void instantaneous_obs::insert_quaternion(
   int pass_number,const osg::Quat& q)
{
   if (retrieve_multiimage_attitude_iter(pass_number)==
       multiple_image_attitude_ptr->end())
   {
      fourvector q4(q.x() , q.y() , q.z() , q.w());
      (*multiple_image_attitude_ptr)[pass_number]=q4;
   }
} 

// ---------------------------------------------------------------------
// Member function insert_scale takes in a pass number along with a
// threevector containing (su,sv,sw) scaling parameters for some
// graphical.  It stores these threevector coordinates in member STL
// map *multiple_image_scale_ptr for later retrieval purposes.

void instantaneous_obs::insert_scale(int pass_number,const threevector& s)
{
   if (retrieve_multiimage_scale_iter(pass_number)==
       multiple_image_scale_ptr->end())
   {
      (*multiple_image_scale_ptr)[pass_number]=s;
   }
} 

// ---------------------------------------------------------------------
// Member function insert_velocity takes in a pass number along with a
// threevector containing (Vu,Vv,Vw) velocity parameters for some
// graphical.  It stores these threevector coordinates in member STL
// map *multiple_image_velocity_ptr for later retrieval purposes.

void instantaneous_obs::insert_velocity(int pass_number,const threevector& V)
{
   if (retrieve_multiimage_velocity_iter(pass_number)==
       multiple_image_velocity_ptr->end())
   {
      (*multiple_image_velocity_ptr)[pass_number]=V;
   }
} 

// ---------------------------------------------------------------------
// Member function insert_score takes in a pass number along with a
// score for some graphical.  It stores the score in member STL map
// *multiple_image_score_ptr for later retrieval purposes.

void instantaneous_obs::insert_score(int pass_number,double score)
{
   if (retrieve_multiimage_score_iter(pass_number)==
       multiple_image_score_ptr->end())
   {
      (*multiple_image_score_ptr)[pass_number]=score;
   }
} 

// ---------------------------------------------------------------------
// Member function insert_index takes in a pass number along with a
// index for some graphical.  It stores the index in member STL map
// *multiple_image_index_ptr for later retrieval purposes.

void instantaneous_obs::insert_index(int pass_number,int index)
{
   if (retrieve_multiimage_index_iter(pass_number)==
       multiple_image_index_ptr->end())
   {
      (*multiple_image_index_ptr)[pass_number]=index;
   }
} 

// ---------------------------------------------------------------------

instantaneous_obs::THREEVECTOR_MAP::iterator 
instantaneous_obs::retrieve_multiimage_coords_iter(
   int pass_number) const
{
   return multiple_image_coords_ptr->find(pass_number);
}

instantaneous_obs::VECTHREEVECTOR_VEC::iterator 
instantaneous_obs::retrieve_multiimage_vertices_iter(
   int pass_number) const
{
   instantaneous_obs::VECTHREEVECTOR_VEC::iterator iter;

   if (pass_number >= 0 && 
       pass_number < int(multiple_image_vertices_vec_ptr->size()))
   {
      return multiple_image_vertices_vec_ptr->begin()+pass_number;
   }
   else
   {
      return multiple_image_vertices_vec_ptr->end();
   }
}

/*
vector<threevector>*
instantaneous_obs::retrieve_multiimage_vertices_vec_ptr(int pass_number) const
{
   if (pass_number >= 0 &&  
   pass_number < multiple_image_vertices_vec_ptr->size())
   {
      return &(multiple_image_vertices_vec_ptr->at(pass_number));
   }
   else
   {
      return NULL;
   }
}
*/

instantaneous_obs::FOURVECTOR_MAP::iterator 
instantaneous_obs::retrieve_multiimage_attitude_iter(
   int pass_number) const
{
   return multiple_image_attitude_ptr->find(pass_number);
}

instantaneous_obs::THREEVECTOR_MAP::iterator 
instantaneous_obs::retrieve_multiimage_scale_iter(
   int pass_number) const
{
   return multiple_image_scale_ptr->find(pass_number);
}

instantaneous_obs::THREEVECTOR_MAP::iterator 
instantaneous_obs::retrieve_multiimage_velocity_iter(
   int pass_number) const
{
   return multiple_image_velocity_ptr->find(pass_number);
}

instantaneous_obs::DOUBLE_MAP::iterator 
instantaneous_obs::retrieve_multiimage_score_iter(
   int pass_number) const
{
   return multiple_image_score_ptr->find(pass_number);
}

instantaneous_obs::INT_MAP::iterator 
instantaneous_obs::retrieve_multiimage_index_iter(
   int pass_number) const
{
   return multiple_image_index_ptr->find(pass_number);
}

// ---------------------------------------------------------------------
bool instantaneous_obs::check_for_pass_entry(int pass_number)
{
   return (multiple_image_coords_ptr->find(pass_number) !=
           multiple_image_coords_ptr->end());
}

// ---------------------------------------------------------------------
// Member function consolidate_image_coords takes in instantaneous_obs
// *other_obs_ptr and loops over its multiple_image_coords_ptr map
// entries.  For any map entry whose pass number does not exist within
// *(this->multiple_image_coords_ptr), this method appends that entry
// onto *(this->multiple_image_coords_ptr).

void instantaneous_obs::consolidate_image_coords(
   instantaneous_obs* other_obs_ptr)
{
//   cout << "inside instantaneous_obs::consolidate_image_coords()" << endl;
   
   THREEVECTOR_MAP* other_multiple_image_coords_ptr=
      other_obs_ptr->get_multiple_image_coords_ptr();

   for (THREEVECTOR_MAP::iterator iter=other_multiple_image_coords_ptr->
           begin(); iter != other_multiple_image_coords_ptr->end(); iter++)
   {
      int pass_number=iter->first;
      if (!check_for_pass_entry(pass_number))
      {
         insert_UVW_coords(pass_number,iter->second);
      }
   }
}

// ==========================================================================
// Coordinate retrieval member functions
// ==========================================================================

// Boolean member function retrieve_UVW_coords performs a brute force
// check whether an entry corresponding to input pass_number exists in
// member STL map *multiple_image_coords_ptr.  If so, it assigns UVW
// information associated with this entry in output threevector p3 and
// returns true.

bool instantaneous_obs::retrieve_UVW_coords(int pass_number,threevector& p3) 
   const
{
//   cout << "inside instantaneous_obs::retrieve_UVW_coords()" << endl;
//   cout << "multi_image_coords.size() = "
//        <<  multiple_image_coords.size() << endl;

   THREEVECTOR_MAP::iterator coords_iter=
      retrieve_multiimage_coords_iter(pass_number);
   if (coords_iter==multiple_image_coords_ptr->end())
   {
      p3=threevector(NEGATIVEINFINITY,NEGATIVEINFINITY,NEGATIVEINFINITY);
      return false;
   }
   else
   {
      p3=coords_iter->second;
      return true;
   }
}

threevector instantaneous_obs::retrieve_UVW_coords(int pass_number) const
{
//   cout << "inside instantaneous_obs::retrieve_UVW_coords()" << endl;
//   cout << "multiple_image_coords.size() = " 
//        << multiple_image_coords.size() << endl;

   threevector p3;
   retrieve_UVW_coords(pass_number,p3);
   return p3;
}

// Boolean method retrieve_transformed_UVW_coords returns false if
// pass corresponding to pass_number is not found.

bool instantaneous_obs::retrieve_transformed_UVW_coords(
   int pass_number,threevector& p_transformed) const
{
   threevector UVW;
   if (retrieve_UVW_coords(pass_number,UVW))
   {
      p_transformed=threevector(UVW.dot(U_hat),UVW.dot(V_hat),
                                UVW.dot(W_hat));
      return true;
   }
   else
   {
      p_transformed=threevector(
         NEGATIVEINFINITY,NEGATIVEINFINITY,NEGATIVEINFINITY);
      return false;
   }
}

threevector instantaneous_obs::retrieve_transformed_UVW_coords(
   int pass_number) const
{
   threevector p_transformed;
   retrieve_transformed_UVW_coords(pass_number,p_transformed);
   return p_transformed;
}

// ---------------------------------------------------------------------
bool instantaneous_obs::retrieve_vertices(
   int pass_number,vector<threevector>& v) const
{
   VECTHREEVECTOR_VEC::iterator vertices_iter=
      retrieve_multiimage_vertices_iter(pass_number);
   if (vertices_iter==multiple_image_vertices_vec_ptr->end())
   {
      return false;
   }
   else
   {
      v=*vertices_iter;
      return true;
   }
}


/*
vector<threevector> instantaneous_obs::retrieve_vertices(int pass_number) 
   const
{
   vector<threevector> v;
   if (!retrieve_vertices(pass_number,v))
   {
      cout << "Error in instantaneous_obs::retrieve_vertices()" << endl;
      exit(-1);
   }
   return v;
}
*/

// ---------------------------------------------------------------------
bool instantaneous_obs::retrieve_quaternion(int pass_number,osg::Quat& q) 
   const
{
   FOURVECTOR_MAP::iterator attitude_iter=
      retrieve_multiimage_attitude_iter(pass_number);
   if (attitude_iter==multiple_image_attitude_ptr->end())
   {
      return false;
   }
   else
   {
      fourvector q4=attitude_iter->second;
      q.set(q4.get(0),q4.get(1),q4.get(2),q4.get(3));
      return true;
   }
}

osg::Quat instantaneous_obs::retrieve_quaternion(int pass_number) const
{
   osg::Quat q;
   if (retrieve_quaternion(pass_number,q))
   {
      return q;
   }
   else
   {
      cout << "Error in instantaneous_obs::retrieve_quaternion()!" << endl;
      exit(-1);
   }
}

// ---------------------------------------------------------------------
bool instantaneous_obs::retrieve_scale(int pass_number,threevector& scale) 
   const
{
   THREEVECTOR_MAP::iterator scale_iter=
      retrieve_multiimage_scale_iter(pass_number);
   if (scale_iter==multiple_image_scale_ptr->end())
   {
      scale=threevector(NEGATIVEINFINITY,NEGATIVEINFINITY,NEGATIVEINFINITY);
      return false;
   }
   else
   {
      scale=scale_iter->second;
      return true;
   }
}

threevector instantaneous_obs::retrieve_scale(int pass_number) const
{
   threevector scale(NEGATIVEINFINITY,NEGATIVEINFINITY,NEGATIVEINFINITY);
   retrieve_scale(pass_number,scale);
   return scale;
}

// ---------------------------------------------------------------------
bool instantaneous_obs::retrieve_velocity(
   int pass_number,threevector& velocity) const
{
   THREEVECTOR_MAP::iterator velocity_iter=
      retrieve_multiimage_velocity_iter(pass_number);
   if (velocity_iter==multiple_image_velocity_ptr->end())
   {
      velocity=threevector(
         NEGATIVEINFINITY,NEGATIVEINFINITY,NEGATIVEINFINITY);
      return false;
   }
   else
   {
      velocity=velocity_iter->second;
      return true;
   }
}

threevector instantaneous_obs::retrieve_velocity(int pass_number) const
{
   threevector velocity(NEGATIVEINFINITY,NEGATIVEINFINITY,NEGATIVEINFINITY);
   retrieve_velocity(pass_number,velocity);
   return velocity;
}

// ---------------------------------------------------------------------
bool instantaneous_obs::retrieve_score(int pass_number,double& score) const
{
   DOUBLE_MAP::iterator score_iter=
      retrieve_multiimage_score_iter(pass_number);
   if (score_iter==multiple_image_score_ptr->end())
   {
      return false;
   }
   else
   {
      score=score_iter->second;
      return true;
   }
}

double instantaneous_obs::retrieve_score(int pass_number) const
{
   double score=NEGATIVEINFINITY;
   retrieve_score(pass_number,score);
   return score;
}

// ---------------------------------------------------------------------
bool instantaneous_obs::retrieve_index(int pass_number,int& index) const
{
   INT_MAP::iterator index_iter=
      retrieve_multiimage_index_iter(pass_number);
   if (index_iter==multiple_image_index_ptr->end())
   {
      return false;
   }
   else
   {
      index=index_iter->second;
      return true;
   }
}

int instantaneous_obs::retrieve_index(int pass_number) const
{
   int index=-1;
   retrieve_index(pass_number,index);
   return index;
}

// ==========================================================================
// Coordinate modification member functions
// ==========================================================================

// Member function change_UVW_coords resets the (U,V,W) coordinates
// within STL map *multiple_image_coords_ptr for the pass labeled by
// input parameter pass_number to input threevector p.

void instantaneous_obs::change_UVW_coords(
   int pass_number,const threevector& p)
{
//   cout << "inside instantaneous_obs::change_UVW_coords()" << endl;
//   cout << "pass_number = " << pass_number << " p3 = " << p << endl;

   THREEVECTOR_MAP::iterator coords_iter=
      retrieve_multiimage_coords_iter(pass_number);
   if (coords_iter==multiple_image_coords_ptr->end())
   {
      insert_UVW_coords(pass_number,p);
   }
   else
   {
      coords_iter->second=p;
   }
}

// ---------------------------------------------------------------------
// Member function change_vertices resets the (U,V,W) coordinates
// within STL map *multiple_image_vertices_ptr for the pass labeled by
// input parameter pass_number to input threevector p.

void instantaneous_obs::change_vertices(
   int pass_number,const vector<threevector>& v)
{
   VECTHREEVECTOR_VEC::iterator vertices_iter=
      retrieve_multiimage_vertices_iter(pass_number);
   if (vertices_iter==multiple_image_vertices_vec_ptr->end())
   {
      insert_vertices(pass_number,v);
   }
   else
   {
      *vertices_iter=v;
   }
}

// ---------------------------------------------------------------------
// Member function change_quaternion resets the quaternion within
// member STL map *multiple_image_attitude_ptr for the pass labeled by
// input parameter pass_number to input quaternion q.

void instantaneous_obs::change_quaternion(int pass_number,const osg::Quat& q)
{
   FOURVECTOR_MAP::iterator attitude_iter=
      retrieve_multiimage_attitude_iter(pass_number);
   if (attitude_iter==multiple_image_attitude_ptr->end())
   {
      insert_quaternion(pass_number,q);
   }
   else
   {
      fourvector q4(q.x() , q.y() , q.z() , q.w());
      attitude_iter->second=q4;
   }
}
 
// ---------------------------------------------------------------------
// Member function change_scale resets the (su,sv,sw) scale parameters
// within STL map *multiple_image_scale_ptr for the pass labeled by
// input parameter pass_number to input threevector s.

void instantaneous_obs::change_scale(int pass_number,const threevector& scale)
{
   THREEVECTOR_MAP::iterator scale_iter=
      retrieve_multiimage_scale_iter(pass_number);
   if (scale_iter != multiple_image_scale_ptr->end())
   {
      scale_iter->second=scale;
   }
   else
   {
      insert_scale(pass_number,scale);
   }
}

// ---------------------------------------------------------------------
// Member function change_velocity resets the (Vu,Vv,Vw) velocity
// parameters within STL map *multiple_image_velocity_ptr for the pass
// labeled by input parameter pass_number to input threevector V.

void instantaneous_obs::change_velocity(
   int pass_number,const threevector& velocity)
{
   THREEVECTOR_MAP::iterator velocity_iter=
      retrieve_multiimage_velocity_iter(pass_number);
   if (velocity_iter != multiple_image_velocity_ptr->end())
   {
      velocity_iter->second=velocity;
   }
   else
   {
      insert_velocity(pass_number,velocity);
   }
}

// ---------------------------------------------------------------------
// Member function change_score resets the score parameter within STL
// vector multiple_image_score for the pass labeled by input parameter
// pass_number to the input score parameter.

void instantaneous_obs::change_score(int pass_number,double score)
{
   DOUBLE_MAP::iterator score_iter=
      retrieve_multiimage_score_iter(pass_number);
   if (score_iter != multiple_image_score_ptr->end())
   {
      score_iter->second=score;
   }
   else
   {
      insert_score(pass_number,score);
   }
}

// ---------------------------------------------------------------------
// Member function change_index resets the index parameter within STL
// vector multiple_image_index for the pass labeled by input parameter
// pass_number to the input index parameter.

void instantaneous_obs::change_index(int pass_number,int index)
{
   INT_MAP::iterator index_iter=
      retrieve_multiimage_index_iter(pass_number);
   if (index_iter != multiple_image_index_ptr->end())
   {
      index_iter->second=index;
   }
   else
   {
      insert_index(pass_number,index);
   }
}

// ==========================================================================
// Coordinate deletion member functions
// ==========================================================================

void instantaneous_obs::delete_all_coords(int pass_number)
{
   delete_UVW_coords(pass_number);
   delete_vertices(pass_number);
   delete_quaternion(pass_number);
   delete_scale(pass_number);
   delete_velocity(pass_number);
   delete_score(pass_number);
   delete_index(pass_number);
}

void instantaneous_obs::delete_UVW_coords(int pass_number)
{
   THREEVECTOR_MAP::iterator iter=
      retrieve_multiimage_coords_iter(pass_number);
   if (iter != multiple_image_coords_ptr->end())
   {
      multiple_image_coords_ptr->erase(iter);
   }
} 

void instantaneous_obs::delete_vertices(int pass_number)
{
   VECTHREEVECTOR_VEC::iterator iter=
      retrieve_multiimage_vertices_iter(pass_number);
   if (iter != multiple_image_vertices_vec_ptr->end())
   {
      multiple_image_vertices_vec_ptr->erase(iter);
   }
} 

void instantaneous_obs::delete_quaternion(int pass_number)
{
   FOURVECTOR_MAP::iterator iter=
      retrieve_multiimage_attitude_iter(pass_number);
   if (iter != multiple_image_attitude_ptr->end())
   {
      multiple_image_attitude_ptr->erase(iter);
   }
} 

void instantaneous_obs::delete_scale(int pass_number)
{
   THREEVECTOR_MAP::iterator iter=
      retrieve_multiimage_scale_iter(pass_number);
   if (iter != multiple_image_scale_ptr->end())
   {
      multiple_image_scale_ptr->erase(iter);
   }
} 

void instantaneous_obs::delete_velocity(int pass_number)
{
   THREEVECTOR_MAP::iterator iter=
      retrieve_multiimage_velocity_iter(pass_number);
   if (iter != multiple_image_velocity_ptr->end())
   {
      multiple_image_velocity_ptr->erase(iter);
   }
} 

void instantaneous_obs::delete_score(int pass_number)
{
   DOUBLE_MAP::iterator iter=retrieve_multiimage_score_iter(pass_number);
   if (iter != multiple_image_score_ptr->end())
   {
      multiple_image_score_ptr->erase(iter);
   }
} 

void instantaneous_obs::delete_index(int pass_number)
{
   INT_MAP::iterator iter=retrieve_multiimage_index_iter(pass_number);
   if (iter != multiple_image_index_ptr->end())
   {
      multiple_image_index_ptr->erase(iter);
   }
} 


