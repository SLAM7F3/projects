// ==========================================================================
// movers_group class member function definitions
// ==========================================================================
// Last updated on 3/5/09; 6/27/09; 10/22/09; 10/26/09; 4/5/14
// ==========================================================================

#include <iostream>
#include <set>
#include <string>
#include <vector>
#include "color/colorfuncs.h"
#include "astro_geo/Clock.h"
#include "astro_geo/geopoint.h"
#include "track/movers_group.h"
#include "general/outputfuncs.h"
#include "geometry/polyline.h"
#include "track/tracks_group.h"

using std::cout;
using std::endl;
using std::flush;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:

void movers_group::allocate_member_objects()
{
   movers_network_ptr=new Network<mover*>(1000);
   movers_map_ptr=new MOVERS_MAP;
}

void movers_group::initialize_member_objects()
{
   n_vehicles=n_ROIs=n_UAVs=n_KOZs=0;
   latest_ROI_ID=latest_KOZ_ID=-1;
   Messenger_ptr=NULL;
}

movers_group::movers_group()
{
   allocate_member_objects();
   initialize_member_objects();
}

movers_group::~movers_group()
{
   delete movers_network_ptr;
   delete movers_map_ptr;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const movers_group& MG)
{
   outstream << endl;

// First print out ROIs:

   outstream << "ROI list:" << endl;
   vector<int> ROI_IDs=MG.get_particular_mover_IDs(mover::ROI);
   for (unsigned int r=0; r<ROI_IDs.size(); r++)
   {
      outstream << ROI_IDs[r] << endl;
   }
   
   outstream << "Vehicle list:" << endl;
   vector<int> Vehicle_IDs=MG.get_particular_mover_IDs(mover::VEHICLE);
   for (unsigned int v=0; v<Vehicle_IDs.size(); v++)
   {
      outstream << Vehicle_IDs[v] << endl;
   }
   
   outstream << "UAV list:" << endl;
   vector<int> UAV_IDs=MG.get_particular_mover_IDs(mover::UAV);
   for (unsigned int u=0; u<UAV_IDs.size(); u++)
   {
      outstream << UAV_IDs[u] << endl;
   }
   
   outstream << "Vehicle-ROI links:" << endl;
   Network<mover*>* movers_network_ptr=MG.get_movers_network_ptr();
   for (unsigned int v=0; v<Vehicle_IDs.size(); v++)
   {
      int curr_vehicle_ID=Vehicle_IDs[v];
      outstream << curr_vehicle_ID << " :  ";
      int r=MG.get_mover_network_index(mover::VEHICLE,curr_vehicle_ID);
      Site<mover*>* curr_site_ptr=movers_network_ptr->get_site_ptr(r);
      vector<int> site_neighbor_indices=curr_site_ptr->get_neighbors();
      for (unsigned int n=0; n<site_neighbor_indices.size(); n++)
      {
         int q=site_neighbor_indices[n];
         const mover* neighbor_mover_ptr=MG.get_mover_ptr(q);
         outstream << neighbor_mover_ptr->get_ID() << " " << flush;
      }
      outstream << endl;
   } // loop over index v labeling Vehicle IDs
 
   outstream << endl;
   return outstream;
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

// Member function get_next_unused_ID loops over all entries within
// *movers_map_ptr.  It returns the smallest integer which does not
// correspond to any mover ID within the map.

int movers_group::get_next_unused_ID() const
{
//   cout << "inside movers_group::get_next_unused_ID()" << endl;

   bool ID_already_exists;
   int next_ID=0;
   do
   {
      ID_already_exists=false;

      for (MOVERS_MAP::iterator iter=movers_map_ptr->begin();
           iter != movers_map_ptr->end(); ++iter)
      {
         int r=iter->second;
         const mover* curr_mover_ptr=get_mover_ptr(r);
         curr_mover_ptr->get_ID();
         if (curr_mover_ptr->get_ID()==next_ID)
         {
            next_ID++;
            ID_already_exists=true;
         }
      }
   }
   while (ID_already_exists);

//   cout << "next_ID = " << next_ID << endl;
   return next_ID;
}

// --------------------------------------------------------------------------
int movers_group::get_next_unused_network_index() const
{
//   cout << "inside movers_group::get_next_unused_network_index()" << endl;

   bool index_already_exists;
   int next_index=0;
   do
   {
      index_already_exists=false;

      for (MOVERS_MAP::iterator iter=movers_map_ptr->begin();
           iter != movers_map_ptr->end(); ++iter)
      {
         int r=iter->second;
         if (r==next_index)
         {
            next_index++;
            index_already_exists=true;
         }
      }
   }
   while (index_already_exists);

//   cout << "next_index = " << next_index << endl;
   return next_index;
}

// --------------------------------------------------------------------------
int movers_group::get_n_particular_movers(mover::MoverType m_type) const
{
//   cout << "inside movers_group::get_n_particular_movers(), m_type = "
//        << m_type << endl;
   if (m_type==mover::VEHICLE)
   {
      return n_vehicles;
   }
   else if (m_type==mover::ROI)
   {
      return n_ROIs;
   }
   else if (m_type==mover::UAV)
   {
      return n_UAVs;
   }
   else // if (m_type==mover::KOZ)
   {
      return n_KOZs;
   }
}

// ==========================================================================
// Mover generation and propagation member functions
// ==========================================================================

mover* movers_group::generate_new_mover(mover::MoverType type,int ID)
{
//   cout << "=====================================================" << endl;
//   cout << "inside movers_group::generate_new_mover, ID = " << ID << endl;

   if (type==mover::ROI)
   {
      cout << "Error in movers_group::generate_new_mover()!" << endl;
      cout << "mover type = ROI" << endl;
      cout << "Should call movers_group::generate_new_ROI() instead!"
           << endl;
      exit(-1);
   }

   if (ID==-1)
   {
      if (type==mover::ROI)
      {
         ID=get_next_unused_ROI_ID();
      }
      else
      {
         ID=get_next_unused_ID();
      }
   }

   mover* curr_mover_ptr=new mover(type,ID);
   insert_mover_into_network(curr_mover_ptr);

   if (type==mover::VEHICLE)
   {
      n_vehicles++;
   }
   else if (type==mover::ROI)
   {
      n_ROIs++;
   }
   else if (type==mover::UAV)
   {
      n_UAVs++;
   }
   else if (type==mover::KOZ)
   {
      n_KOZs++;
   }

   return curr_mover_ptr;
}

// ---------------------------------------------------------------------
// This specialized version of generate_new_vehicle() associates an
// dynamic track with a new instantiated vehicle mover.

mover* movers_group::generate_new_vehicle(track* vehicle_track_ptr,int ID)
{
//   cout << "=====================================================" << endl;
//   cout << "inside movers_group::generate_new_vehicle, input ID = " << ID 
//        << endl;

   if (ID==-1) ID=get_next_unused_ROI_ID();
//   cout << "Final ID = " << ID << endl;

   mover* curr_mover_ptr=new mover(mover::VEHICLE,ID);
   insert_mover_into_network(curr_mover_ptr);

   vehicle_track_ptr->set_spatially_fixed_flag(false);
   vehicle_track_ptr->set_description("Ground Target");
   vehicle_track_ptr->set_broadcast_contents_flag(true);
   curr_mover_ptr->set_track_ptr(vehicle_track_ptr);

   n_vehicles++;
//   print_n_ROIs_vehicles_UAVs_and_KOZs();
   return curr_mover_ptr;
}

// ---------------------------------------------------------------------
// This specialized version of generate_new_mover() instantiates a
// spatially fixed track to accompany the new ROI mover.

mover* movers_group::generate_new_ROI(
   tracks_group* spatially_fixed_tracks_group_ptr,int ID)
{
//   cout << "=====================================================" << endl;
//   cout << "inside movers_group::generate_new_ROI, input ID = " << ID << endl;

   if (ID==-1) ID=get_next_unused_ROI_ID();
//   cout << "Final ID = " << ID << endl;

   mover* curr_mover_ptr=new mover(mover::ROI,ID);
   insert_mover_into_network(curr_mover_ptr);

   track* curr_ROI_track_ptr=spatially_fixed_tracks_group_ptr->
      generate_new_track();
   curr_ROI_track_ptr->set_spatially_fixed_flag(true);
   curr_ROI_track_ptr->set_description("Ground Target");
   curr_ROI_track_ptr->set_broadcast_contents_flag(true);
   curr_mover_ptr->set_track_ptr(curr_ROI_track_ptr);

   n_ROIs++;
   print_n_ROIs_vehicles_UAVs_and_KOZs();
   return curr_mover_ptr;
}

// ---------------------------------------------------------------------
// This specialized version of generate_new_mover() instantiates a
// spatially fixed track to accompany the new KOZ mover:

mover* movers_group::generate_new_KOZ(
   tracks_group* KOZ_tracks_group_ptr,int ID)
{
//   cout << "=====================================================" << endl;
//   cout << "inside movers_group::generate_new_KOZ, input ID = " << ID << endl;

   if (ID==-1) ID=get_next_unused_KOZ_ID();
//   cout << "Final ID = " << ID << endl;

   mover* curr_mover_ptr=new mover(mover::KOZ,ID);
   insert_mover_into_network(curr_mover_ptr);

   track* curr_KOZ_track_ptr=KOZ_tracks_group_ptr->generate_new_track();
   curr_KOZ_track_ptr->set_spatially_fixed_flag(true);
   curr_KOZ_track_ptr->set_description("Keep Out Zone");
   curr_KOZ_track_ptr->set_broadcast_contents_flag(true);
   curr_mover_ptr->set_track_ptr(curr_KOZ_track_ptr);

   n_KOZs++;

   print_n_ROIs_vehicles_UAVs_and_KOZs();
   return curr_mover_ptr;
}

// ---------------------------------------------------------------------
// Member function delete_mover removes the input mover from
// *movers_network_ptr and *movers_map_ptr.  It then destroys the
// dynamically generated mover itself.  This method also issues
// appropriate DELETE_EDGE and DELETE_VERTEX ActiveMQ messages.

void movers_group::delete_mover(mover::MoverType mover_type,int ID)
{
//   cout << "inside movers_group::delete_mover()" << endl;
   int r=get_mover_network_index(mover_type,ID);

   mover* curr_mover_ptr=get_mover_ptr(r);
   if (curr_mover_ptr==NULL)
   {
      cout << "Error in movers_group::delete_mover()!!!" << endl;
      cout << "curr_mover_ptr = NULL!!!" << endl;
      return;
   }

   if (curr_mover_ptr->get_MoverType()==mover::ROI)
   {
      n_ROIs--;
   }
   else if (curr_mover_ptr->get_MoverType()==mover::VEHICLE)
   {
      n_vehicles--;
   }
   else if (curr_mover_ptr->get_MoverType()==mover::UAV)
   {
      n_UAVs--;
   }
   else if (curr_mover_ptr->get_MoverType()==mover::KOZ)
   {
      n_KOZs--;
   }

// Retrieve current mover's neighbors.  Issue delete edge messages
// followed by a delete current mover vertex message.  As of 9/12/08,
// we do NOT broadcast any messages to the GoogleEarth channel.  This
// is a hack...

//   cout << "Messenger_ptr->get_topicName() = "
//        << Messenger_ptr->get_topicName() << endl;
   if (Messenger_ptr != NULL &&
       Messenger_ptr->get_topicName() != "GoogleEarth")
   {
      Site<mover*>* curr_site_ptr=movers_network_ptr->get_site_ptr(r);
      vector<int> site_neighbor_indices=curr_site_ptr->get_neighbors();
      for (unsigned int n=0; n<site_neighbor_indices.size(); n++)
      {
         int q=site_neighbor_indices[n];
         mover* neighbor_mover_ptr=get_mover_ptr(q);
         issue_delete_edge_message(curr_mover_ptr,neighbor_mover_ptr);
      }
      issue_delete_vertex_message(curr_mover_ptr);

   } // topicName != GoogleEarth conditional

   movers_network_ptr->delete_single_site(r);
   movers_map_ptr->erase(twovector(mover_type,ID));
   delete curr_mover_ptr;

   print_n_ROIs_vehicles_UAVs_and_KOZs();
}

// ---------------------------------------------------------------------
// Member function get_particular_mover_IDs() iterates over all movers
// within *movers_network_ptr.  It returns the IDs of those movers
// matching the specified input type within an output STL vector.

vector<int> movers_group::get_particular_mover_IDs(mover::MoverType m_type) 
   const
{
//   cout << "inside movers_group::get_particular_mover_IDs()" << endl;
//   cout << "movers_map_ptr->size() = " << movers_map_ptr->size() << endl;
   
   vector<int> particular_mover_IDs;

   for (MOVERS_MAP::iterator iter=movers_map_ptr->begin();
        iter != movers_map_ptr->end(); ++iter)
   {
      int r=iter->second;
      const mover* curr_mover_ptr=get_mover_ptr(r);
      
      if (curr_mover_ptr->get_MoverType()==m_type)
      {
         particular_mover_IDs.push_back(curr_mover_ptr->get_ID());
//         cout << "iteration r = " << r 
//              << " particular mover ID = " << particular_mover_IDs.back() 
//              << " curr_mover_ptr = " << curr_mover_ptr 
//              << endl;
      }
   }
   return particular_mover_IDs;
}

// ---------------------------------------------------------------------
// Member function purge_all_particular_movers loops over all mover
// entries within *movers_map_ptr of type m_type and calls
// delete_mover for each one.

void movers_group::purge_all_particular_movers(mover::MoverType m_type)
{
//   cout << "inside movers_group::purge_all_particular_movers()" << endl;

// Loop over particular_mover_IDs vector and delete each corresponding
// particular mover:

   vector<int> particular_mover_IDs=get_particular_mover_IDs(m_type);
   for (unsigned int v=0; v<particular_mover_IDs.size(); v++)
   {
//      cout << "v = " << v << endl;
      delete_mover(m_type,particular_mover_IDs[v]);
   }

   if (m_type==mover::ROI)
   {
      latest_ROI_ID=-1;
   }
   else if (m_type==mover::KOZ)
   {
      latest_KOZ_ID=-1;
   }
}

// --------------------------------------------------------------------------
int movers_group::get_next_unused_ROI_ID()
{
//   cout << "inside movers_group::get_next_unused_ROI_ID()" << endl;

   latest_ROI_ID++;
   return latest_ROI_ID;
}

// --------------------------------------------------------------------------
int movers_group::get_next_unused_KOZ_ID()
{
//   cout << "inside movers_group::get_next_unused_KOZ_ID()" << endl;

   latest_KOZ_ID++;
   return latest_KOZ_ID;
}

// ---------------------------------------------------------------------
void movers_group::print_n_ROIs_vehicles_UAVs_and_KOZs()
{
   cout << "n_vehicles in network = " 
        << get_n_particular_movers(mover::VEHICLE) << endl;
   cout << "n_ROIs in network = " 
        << get_n_particular_movers(mover::ROI) << endl;
   cout << "n_UAVs in network = " 
        << get_n_particular_movers(mover::UAV) << endl;
   cout << "n_KOZs in network = " 
        << get_n_particular_movers(mover::KOZ) << endl;
   cout << endl;
}

// ---------------------------------------------------------------------
// Member function get_MoverType_string returns an output string
// corresponding to the input mover's MoverType.

string movers_group::get_MoverType_string(mover* mover_ptr)
{
   string value="";
   if (mover_ptr->get_MoverType()==mover::ROI)
   {
      value="ROI";
   }
   else if (mover_ptr->get_MoverType()==mover::VEHICLE)
   {
      value="VEHICLE";
   }
   else if (mover_ptr->get_MoverType()==mover::UAV)
   {
      value="UAV";
   }
   else if (mover_ptr->get_MoverType()==mover::KOZ)
   {
      value="KOZ";
   }
   return value;
}

// ==========================================================================
// Mover network member functions
// ==========================================================================

// Member function insert_mover_into_network inserts the input mover
// *curr_mover_ptr into *movers_network_ptr with a network index
// corresponding to the network's current size.  It also records the
// network index as a dependent variable within the STL map member
// *movers_map_ptr in order to allow for future fast retrieval given
// independent MoverType and mover ID inputs.

void movers_group::insert_mover_into_network(mover* curr_mover_ptr)
{
//   cout << "inside movers_group::insert_mover_into_network()" << endl;
//   cout << "curr_mover_ptr->get_ID() = " << curr_mover_ptr->get_ID()
//        << endl;
//   cout << "movers_network_ptr->size() = " << movers_network_ptr->size()
//        << endl;
   int next_unused_network_index=get_next_unused_network_index();

   movers_network_ptr->insert_site(
      next_unused_network_index,Site<mover*>(curr_mover_ptr));
   twovector indep_var(
      curr_mover_ptr->get_MoverType(),curr_mover_ptr->get_ID());
   (*movers_map_ptr)[indep_var]=next_unused_network_index;
}

// ---------------------------------------------------------------------
// Member function get_mover_network_index takes in mover type and ID.
// It returns the index for the node within *movers_network_ptr
// corresponding to the requested mover if it exists in the network.
// Otherwise, this method returns -1.

int movers_group::get_mover_network_index(
   mover::MoverType mover_type,int mover_ID) const
{
//   cout << "inside movers_group::get_mover_network_index()" << endl;
   
   int r=-1;
//   cout << "movers_map_ptr = " << movers_map_ptr << endl;
//   cout << "mover_type = " << mover_type << " mover_ID = " << mover_ID
//        << endl;
   
   MOVERS_MAP::iterator mover_iter=movers_map_ptr->find(
      twovector(mover_type,mover_ID));
   if (mover_iter != movers_map_ptr->end())
   {
      r=mover_iter->second;
   }
//   cout << "r = " << r << endl;
//   cout << "At end of movers_group::get_mover_network_index()" << endl;
   return r;
}

// ---------------------------------------------------------------------
mover* movers_group::get_mover_ptr(mover::MoverType mover_type,int mover_ID)
{
//   cout << "inside movers_group::get_mover_ptr, mover_ID = " << mover_ID
//        << endl;
   int r=get_mover_network_index(mover_type,mover_ID);
//   cout << "index r = " << r << endl;
   return get_mover_ptr(r);
}

// ---------------------------------------------------------------------
// Member function add_mover_network_link adds a symmetric link
// between two mover sites within *movers_network_ptr based upon their
// input types and IDs.

void movers_group::add_mover_network_link(
   mover::MoverType first_mover_type,int first_mover_ID,
   mover::MoverType second_mover_type,int second_mover_ID)
{
   int r1=get_mover_network_index(first_mover_type,first_mover_ID);
   int r2=get_mover_network_index(second_mover_type,second_mover_ID);
//   cout << "First mover network index = " << r1 << endl;
//   cout << "Second mover network index = " << r2 << endl;
   
   if (r1 >=0 && r2 >= 0)
   {
      movers_network_ptr->add_symmetric_link(r1,r2);
   }
}

// ==========================================================================
// ROI-vehicle association member functions
// ==========================================================================

// Member function add_ROI_vehicle_network_link

void movers_group::add_ROI_vehicle_network_link(int ROI_ID,int vehicle_ID)
{
//   cout << "inside movers_group::add_ROI_vehicle_network_link()" << endl;
//   cout << "ROI_ID = " << ROI_ID << " vehicle_ID = " << vehicle_ID << endl;
   add_mover_network_link(
      mover::ROI,ROI_ID,
      mover::VEHICLE,vehicle_ID);
}

// ---------------------------------------------------------------------
// Member function add_vehicle_vehicle_network_link

void movers_group::add_vehicle_vehicle_network_link(
   int first_vehicle_ID,int second_vehicle_ID)
{
   add_mover_network_link(
      mover::VEHICLE,first_vehicle_ID,
      mover::VEHICLE,second_vehicle_ID);
}

// ---------------------------------------------------------------------
// Member function add_ROI_ROI_network_link

void movers_group::add_ROI_ROI_network_link(
   int first_ROI_ID,int second_ROI_ID)
{
   add_mover_network_link(
      mover::ROI,first_ROI_ID,
      mover::ROI,second_ROI_ID);
}

// ---------------------------------------------------------------------
// Member function associate_vehicles_with_ROI takes in
// *tracks_group_ptr which is assumed to hold vehicle tracks passing
// through the last entered Region of Interest (ROI).  This method
// loops over all the vehicle tracks, extracts their Bluegrass labels
// as vehicle IDs and forms ROI-vehicle links within
// *movers_network_ptr.

void movers_group::associate_vehicles_with_ROI(
   int ROI_ID,tracks_group* tracks_group_ptr)
{
//   cout << "inside movers_group::associate_vehicle_with_ROI()" << endl;
//   cout << "ROI_ID = " << ROI_ID << endl;

   vector<track*> tracks_to_destroy;
   vector<track*> track_ptrs=tracks_group_ptr->get_all_track_ptrs();
   for (unsigned int t=0; t<track_ptrs.size(); t++)
   {
      track* curr_track_ptr=track_ptrs[t];
//      int curr_track_ID=curr_track_ptr->get_ID();
      int vehicle_label_number=curr_track_ptr->get_label_ID();
//      cout << "t = " << t 
//           << " track ID = " << curr_track_ID
//           << " vehicle_label_number = " << vehicle_label_number << endl;
//      cout << "curr_track_ptr = " << curr_track_ptr << endl;

// Check whether mover associated with vehicle_label_number already
// exists within *movers_network_ptr.  If not, generate a new vehicle
// mover and issue an ADD_VERTEX ActiveMQ message:

      if (get_mover_network_index(mover::VEHICLE,vehicle_label_number)==-1)
      {
         mover* curr_vehicle_ptr=generate_new_mover(
            mover::VEHICLE,vehicle_label_number);
         colorfunc::RGB curr_RGB=curr_track_ptr->get_RGB_color();
//         cout << "curr_RGB = " << curr_RGB << endl;
         curr_vehicle_ptr->set_RGB_color(curr_RGB);
         issue_add_vertex_message(curr_vehicle_ptr);
      }
      else
      {
         tracks_to_destroy.push_back(curr_track_ptr);
      }
      
      add_ROI_vehicle_network_link(ROI_ID,vehicle_label_number);

// Generate ActiveMQ "ADD_EDGE" message:

      mover* mover1_ptr=get_mover_ptr(mover::ROI,ROI_ID);
      mover* mover2_ptr=get_mover_ptr(mover::VEHICLE,vehicle_label_number);

      issue_add_edge_message(mover1_ptr,mover2_ptr);
   } // loop over index t labeling vehicle tracks

// Destroy tracks associated with movers that already existed within
// *movers_network_ptr so that they're not drawn twice in the map
// client:

   for (unsigned int t=0; t<tracks_to_destroy.size(); t++)
   {
      track* track_to_destroy_ptr=tracks_to_destroy[t];
//      cout << "t = " << t 
//           << " Destroying track ID = " << track_to_destroy_ptr->get_ID()
//           << " Destroying track label = " 
//           << track_to_destroy_ptr->get_label_ID() << endl;
      tracks_group_ptr->destroy_track(track_to_destroy_ptr);
   }

//   cout << "*movers_network_ptr = "
//        << *movers_network_ptr << endl;
   print_n_ROIs_vehicles_UAVs_and_KOZs();
}

// ---------------------------------------------------------------------
// This next overloaded version of associate_vehicles_with_ROI takes
// in some ROI's ID along with a vector of vehicle label IDs.  It adds
// network links between the ROI and the vehicles.  This method also
// issues add_edge ActiveMQ messages.

void movers_group::associate_vehicles_with_ROI(
   int ROI_ID,const vector<int>& vehicle_label_IDs)
{
//   cout << "inside movers_group::associate_vehicle_with_ROI(ROI_ID,vector<int>)" << endl;
//   cout << "ROI_ID = " << ROI_ID << endl;

   for (unsigned int t=0; t<vehicle_label_IDs.size(); t++)
   {
      int vehicle_label_number=vehicle_label_IDs[t];
//      cout << "t = " << t << " vehicle_label_number = " 
//           << vehicle_label_number << endl;

      add_ROI_vehicle_network_link(ROI_ID,vehicle_label_number);

// Generate ActiveMQ "ADD_EDGE" message:

      mover* mover1_ptr=get_mover_ptr(mover::ROI,ROI_ID);
      mover* mover2_ptr=get_mover_ptr(mover::VEHICLE,vehicle_label_number);
      issue_add_edge_message(mover1_ptr,mover2_ptr);
   } // loop over index t labeling vehicle labels

//   cout << "*movers_network_ptr = " << *movers_network_ptr << endl;
}

// ==========================================================================
// Ground mover-UAV association member functions
// ==========================================================================

// Member function check_for_UAV_ground_target_encounters() loops over
// every ROI or VEHICLE within *this and UAV mover within input
// *UAV_movers_group_ptr.  After extracting their XY positions, this
// method computes their 2D separation.  If the separation distance is
// less than encounter_distance, the ROI or VEHICLE mover's
// previously_encounter_flag is set equal to true.

void movers_group::check_for_UAV_ground_target_encounters(
   double curr_t,movers_group* UAV_movers_group_ptr,
   mover::MoverType mover_type)
{
//   cout << "inside movers_group::check_for_UAV_ground_target_encounters()" 
//        << endl;
//   cout << "mover_type = " << mover_type << endl;

   if (UAV_movers_group_ptr->get_n_UAVs()==0) return;

   const double encounter_distance=250; // meters
   
   vector<int> ground_target_IDs=get_particular_mover_IDs(mover_type);
//   cout << "ground_target_IDs.size() = " << ground_target_IDs.size() << endl;
   vector<int> UAV_IDs=UAV_movers_group_ptr->get_particular_mover_IDs(
      mover::UAV);
////   cout << "UAV_movers_group_ptr = " << UAV_movers_group_ptr << endl;
//   cout << "*UAV_movers_group_ptr = " << *UAV_movers_group_ptr;
//   cout << "UAV_IDs.size() = " << UAV_IDs.size() << endl;

   for (unsigned int r=0; r<ground_target_IDs.size(); r++)
   {
//      cout << "r = " << r << " ground_target_ID = " 
//           << ground_target_IDs[r] << endl;

      int curr_ground_target_ID=ground_target_IDs[r];
      mover* curr_ground_target_mover_ptr=get_mover_ptr(
         mover_type,curr_ground_target_ID);
      
      if (curr_ground_target_mover_ptr->get_previously_encountered_flag())
      {
//         cout << "curr ground target mover previously encountered" << endl;
         continue;
      }

      track* ground_target_track_ptr=curr_ground_target_mover_ptr->
         get_track_ptr();

      if (ground_target_track_ptr==NULL) continue;

//      cout << "r = " << r 
//           << " ground_target_ID = " << ground_target_IDs[r] 
//           << " mover_ptr->get_ID() = " 
//           << curr_ground_target_mover_ptr->get_ID()
//           << " track_ptr->get_ID() = " 
//           << ground_target_track_ptr->get_ID()
//           << " track_ptr->get_label_ID() = " 
//           << ground_target_track_ptr->get_label_ID()
//           << endl;

      threevector ground_target_posn;
      ground_target_track_ptr->get_interpolated_posn(
         curr_t,ground_target_posn);

      twovector ground_target_XY(ground_target_posn);
      for (unsigned int u=0; u<UAV_movers_group_ptr->get_n_UAVs(); u++)
      {
//         cout << "u = " << u << " UAV_ID = " << UAV_IDs[u] << endl;
         mover* curr_UAV_mover_ptr=UAV_movers_group_ptr->get_mover_ptr(
            mover::UAV,UAV_IDs[u]);

         track* UAV_track_ptr=curr_UAV_mover_ptr->get_track_ptr();
         threevector UAV_posn;
         if (UAV_track_ptr->get_interpolated_posn(curr_t,UAV_posn))
         {
            twovector UAV_XY(UAV_posn);
//            cout << "ID = " << UAV_IDs[u] 
//                 << " UAV_X = " << UAV_XY.get(0)
//                 << " UAV_Y = " << UAV_XY.get(1) << endl;
               
            double delta_XY=(UAV_XY-ground_target_XY).magnitude();
//            cout << "r = " << r 
//                 << " ground tgt ID = " << curr_ground_target_ID
//                 << " delta_XY = " << delta_XY << endl;
            
            if (delta_XY < encounter_distance)
            {
               curr_ground_target_mover_ptr->
                  set_previously_encountered_flag(true);
               if (mover_type==mover::VEHICLE)
               {
                  cout << "Vehicle " << curr_ground_target_ID
                       << " encountered by UAV " << UAV_IDs[u] << endl;
                  encountered_vehicle_IDs.push_back(curr_ground_target_ID);
               }
               else if (mover_type==mover::ROI)
               {
                  cout << "ROI " << curr_ground_target_ID 
                       << " encountered by UAV " << UAV_IDs[u] << endl;
                  encountered_ROI_IDs.push_back(curr_ground_target_ID);
               }
            } // delta_XY < encounter_distance conditional
         } // get UAV coords conditional
      } // loop over u index labeling UAVs

   } // loop over index r labeling ground_target IDs
}

// ---------------------------------------------------------------------
// Member function compute_passed_ground_targets() takes in some
// current UAV position along with an original UAV flight path stored
// within the mover for UAV labeled by input UAV_ID within
// *UAV_movers_group_ptr.  It first finds point along the original
// flight path polyline lying closest to the current UAV's position
// and computes a fractional UAV polyline distance.  This method
// subsequently loops over all ROI vertex locations along the original
// flight path and computes their fractional polyline distances.  We
// assume that any ROI whose fractional distance is less than the
// current UAV's has previously been overflown.  The IDs for such ROIs
// are added to the encountered_ROI_IDs STL vector member.

void movers_group::compute_passed_ground_targets(
   double curr_t,int UAV_ID,const threevector& curr_UAV_posn,
   movers_group* UAV_movers_group_ptr,
   mover::MoverType mover_type)
{
//   cout << "inside movers_group::compute_passed_ground_targets()" << endl;
//   cout << "mover_type = " << mover_type << endl;
    
// Recover UAV's original flight path:

   mover* mover_ptr=UAV_movers_group_ptr->get_mover_ptr(mover::UAV,UAV_ID);
   track* orig_track_ptr=mover_ptr->get_orig_track_ptr();
   if (orig_track_ptr==NULL)
   {
      cout << "Error in movers_group::compute_passed_ground_targets()!"
           << endl;
      cout << "orig_track_ptr = NULL!" << endl;
      return;
   }
      
   vector<threevector> orig_track_posns=orig_track_ptr->get_posns();
   polyline orig_track_polyline(orig_track_posns);
   
// Compute UAV's current fractional position along original flight path:
  
   threevector closest_point_on_polyline;
   orig_track_polyline.min_distance_to_point(
      curr_UAV_posn,closest_point_on_polyline);
   double curr_UAV_frac_posn=orig_track_polyline.frac_distance_along_polyline(
      closest_point_on_polyline);
//   cout << "curr_UAV_frac_posn = " << curr_UAV_frac_posn << endl;

// Find ground targets with flight path fractions less than UAV's:
   
   vector<int> ground_target_IDs=get_particular_mover_IDs(mover_type);
   for (unsigned int r=0; r<ground_target_IDs.size(); r++)
   {

      int curr_ground_target_ID=ground_target_IDs[r];
      mover* curr_ground_target_mover_ptr=get_mover_ptr(
         mover_type,curr_ground_target_ID);
      
      if (curr_ground_target_mover_ptr->get_previously_encountered_flag())
      {
//         cout << "curr ground target mover previously encountered" << endl;
         continue;
      }

      track* ground_target_track_ptr=curr_ground_target_mover_ptr->
         get_track_ptr();

      if (ground_target_track_ptr==NULL) continue;

      threevector ground_target_posn;
      ground_target_track_ptr->get_interpolated_posn(
         curr_t,ground_target_posn);
      orig_track_polyline.min_distance_to_point(
         ground_target_posn,closest_point_on_polyline);
      double curr_target_frac_posn=orig_track_polyline.
         frac_distance_along_polyline(
            closest_point_on_polyline);
      
      if (curr_target_frac_posn < curr_UAV_frac_posn)
      {
         int curr_ground_target_ID=ground_target_IDs[r];
         cout << "r = " << r << " ground_target_ID = " 
              << curr_ground_target_ID << endl;
         
         if (mover_type==mover::VEHICLE)
         {
            cout << "Vehicle " << curr_ground_target_ID
                 << " encountered by UAV " << UAV_ID << endl;
            encountered_vehicle_IDs.push_back(curr_ground_target_ID);
         }
         else if (mover_type==mover::ROI)
         {
            cout << "ROI " << curr_ground_target_ID 
                 << " encountered by UAV " << UAV_ID << endl;
            encountered_ROI_IDs.push_back(curr_ground_target_ID);
         }
      }
   } // loop over index r labeling ground targets

}

// ==========================================================================
// Ground target path generation member functions
// ==========================================================================

// Member function generate_ground_target_posns takes in an STL vector
// containing integer ROI or vehicle IDs.  It returns an STL vector
// filled with the ROIs' or vehicles' corresponding positions.

vector<threevector> movers_group::generate_ground_target_posns(
   double curr_t,const vector<int>& ground_target_IDs,
   mover::MoverType mover_type)
{
//   cout << "inside movers_group::generate_ground_target_posns()" << endl;
//   cout << "mover_type = " << mover_type << endl;

   threevector interpolated_posn;
   vector<threevector> ground_target_posns;
   for (unsigned int i=0; i<ground_target_IDs.size(); i++)
   {
      mover* ground_target_mover_ptr=get_mover_ptr(
         mover_type,ground_target_IDs[i]);

      if (ground_target_mover_ptr==NULL) continue;
      track* track_ptr=ground_target_mover_ptr->get_track_ptr();
      if (track_ptr==NULL) continue;

      if (curr_t <= track_ptr->get_earliest_time())
      {
         interpolated_posn=track_ptr->get_earliest_posn();
      }
      else if (curr_t >= track_ptr->get_latest_time())
      {
         interpolated_posn=track_ptr->get_latest_posn();
      }
      else
      {
         track_ptr->get_interpolated_posn(curr_t,interpolated_posn);
      }
   
      ground_target_posns.push_back(interpolated_posn);

//      cout << "i = " << i << " ground_target_posn = " 
//           << ground_target_posns.back() << endl;
   } // loop over index i labeing ground_target_IDs

   return ground_target_posns;
}

// ==========================================================================
// Message handling member functions
// ==========================================================================

// Member function add_mover_to_outgoing_queue

void movers_group::add_mover_to_outgoing_queue(mover* mover_ptr)
{
//   cout << "inside movers_group::add_mover_to_outgoing_queue()" << endl;
   movers_queue.push_back(mover_ptr);
//   cout << "movers_queue.size() = " << movers_queue.size() << endl;
}

// ---------------------------------------------------------------------
// Member function issue_add_vertex_message takes in *mover_ptr and
// generates an ActiveMQ "ADD_VERTEX" message.  The mover's type, ID
// and RGB color are encoded as key-value string pair properties.

void movers_group::issue_add_vertex_message(mover* mover_ptr)
{
//   cout << "inside movers_group::issue_add_vertex_message()" << endl;
//   cout << "*mover_ptr = " << *mover_ptr << endl;

   if (Messenger_ptr==NULL ||
       !Messenger_ptr->connected_to_broker_flag()) return;
   
// Recall that ActiveMQ messages consist of a single command string
// along with an STL vector of key-value string pair properties:

   string command,key,value;
   typedef pair<string,string> property;
   vector<property> properties;

// Adding/selecting vertex example:

   command="ADD_VERTEX";
//   command="SELECT_VERTEX";
 
   key="TYPE";
   value=get_MoverType_string(mover_ptr);

/*   
   if (mover_ptr->get_MoverType()==mover::ROI)
   {
      value="ROI";
   }
   else if (mover_ptr->get_MoverType()==mover::VEHICLE)
   {
      value="VEHICLE";
   }
   else if (mover_ptr->get_MoverType()==mover::KOZ)
   {
      value="KOZ";
   }
*/
   properties.push_back(property(key,value));

   key="ID";
   value=stringfunc::number_to_string(mover_ptr->get_ID());
   properties.push_back(property(key,value));

   string annotation_label=mover_ptr->get_annotation_label();
   if (annotation_label.size() > 0)
   {
      key="ANNOTATION_LABEL";
      value=annotation_label;
      properties.push_back(property(key,value));
   }
   
// If the current mover has valid relative size, average time duration
// or RGB metadata, include that infomration as properties in the
// ActiveMQ message:

   const double TINY_NEG=-0.00001;

/*
   double relative_size=mover_ptr->get_relative_size();
   if (relative_size > TINY_NEG)
   {
      key="RELATIVE_SIZE";
      value=stringfunc::number_to_string(relative_size);
      properties.push_back(property(key,value));
   }
*/

   double avg_time_duration=mover_ptr->get_avg_time_duration();
   if (avg_time_duration > TINY_NEG)
   {
      key="AVG_TIME_DURATION";
      value=stringfunc::number_to_string(avg_time_duration);
      properties.push_back(property(key,value));
   }

   colorfunc::RGB curr_RGB=mover_ptr->get_RGB_color();
/*
   double red=curr_RGB.first;
   double green=curr_RGB.second;
   double blue=curr_RGB.third;
//   cout << "red = " << red << " green = " << green << " blue = " << blue
//        << endl;

   if (red > TINY_NEG && green > TINY_NEG && blue > TINY_NEG)
   {
      key="RED";
      value=stringfunc::number_to_string(red);
      properties.push_back(property(key,value));

      key="GREEN";
      value=stringfunc::number_to_string(green);
      properties.push_back(property(key,value));

      key="BLUE";
      value=stringfunc::number_to_string(blue);
      properties.push_back(property(key,value));
   }
*/

   key="RGB COLOR";
   value=stringfunc::number_to_string(curr_RGB.first)+" ";
   value += stringfunc::number_to_string(curr_RGB.second)+" ";
   value += stringfunc::number_to_string(curr_RGB.third);
   properties.push_back(property(key,value));

//   for (unsigned int p=0; p<properties.size(); p++)
//   {
//      cout << "p = " << p
//           << " key = " << properties[p].first
//           << " value = " << properties[p].second << endl;
//   }

//   cout << "Messenger_ptr->get_topicName() = "
//        << Messenger_ptr->get_topicName() << endl;

   Messenger_ptr->sendTextMessage(command,properties);
}

// ---------------------------------------------------------------------
// Member function issue_add_edge_message takes in *mover1_ptr and
// *mover2_ptr and generates an ActiveMQ "ADD_EDGE" message.  The two
// movers types and IDs are encoded as key-value string pair
// properties.

void movers_group::issue_add_edge_message(
   mover* mover1_ptr,mover* mover2_ptr)
{
//   cout << "inside movers_group::issue_add_edge_message()" << endl;

   if (mover1_ptr==NULL || mover2_ptr==NULL)
   {
//      cout << "Error in movers_group::issue_add_edge_message()!" << endl;
//      cout << "mover1_ptr = " << mover1_ptr 
//           << " mover2_ptr = " << mover2_ptr << endl;
//      outputfunc::enter_continue_char();
      return;
   }

   if (Messenger_ptr==NULL ||
       !Messenger_ptr->connected_to_broker_flag()) return;

   string command,key,value;
   typedef pair<string,string> property;
   vector<property> properties;

   command="ADD_EDGE";
//   command="SELECT_EDGE";

   key="TYPE1";
   value=get_MoverType_string(mover1_ptr);

/*
   if (mover1_ptr->get_MoverType()==mover::ROI)
   {
      value="ROI";
   }
   else if (mover1_ptr->get_MoverType()==mover::VEHICLE)
   {
      value="VEHICLE";
   }
*/
   properties.push_back(property(key,value));

   key="ID1";
   value=stringfunc::number_to_string(mover1_ptr->get_ID());
   properties.push_back(property(key,value));

   key="TYPE2";
   value=get_MoverType_string(mover2_ptr);
/*
   if (mover2_ptr->get_MoverType()==mover::ROI)
   {
      value="ROI";
   }
   else if (mover2_ptr->get_MoverType()==mover::VEHICLE)
   {
      value="VEHICLE";
   }
*/
   properties.push_back(property(key,value));

   key="ID2";
   value=stringfunc::number_to_string(mover2_ptr->get_ID());
   properties.push_back(property(key,value));

   Messenger_ptr->sendTextMessage(command,properties);
}

// ---------------------------------------------------------------------
// Member function issue_delete_vertex_message takes in *mover_ptr and
// generates an ActiveMQ "DELETE_VERTEX" message.  The mover's type and
// ID are encoded as key-value string pair properties.

void movers_group::issue_delete_vertex_message(mover* mover_ptr)
{
//   cout << "inside movers_group::issue_delete_vertex_message()" << endl;
//   cout << "*mover_ptr = " << *mover_ptr << endl;
//   cout << "Messenger_ptr = " << Messenger_ptr << endl;

   if (Messenger_ptr==NULL ||
       !Messenger_ptr->connected_to_broker_flag()) return;

// Recall that ActiveMQ messages consist of a single command string
// along with an STL vector of key-value string pair properties:

   string command,key,value;
   typedef pair<string,string> property;
   vector<property> properties;

// Deleting/selecting vertex example:

   command="DELETE_VERTEX";
//      command="SELECT_VERTEX";

   key="TYPE";
   value=get_MoverType_string(mover_ptr);
/*
   if (mover_ptr->get_MoverType()==mover::ROI)
   {
      value="ROI";
   }
   else if (mover_ptr->get_MoverType()==mover::VEHICLE)
   {
      value="VEHICLE";
   }
   else if (mover_ptr->get_MoverType()==mover::UAV)
   {
      value="UAV";
   }
   else if (mover_ptr->get_MoverType()==mover::KOZ)
   {
      value="KOZ";
   }
*/

   properties.push_back(property(key,value));

   key="ID";
   value=stringfunc::number_to_string(mover_ptr->get_ID());
   properties.push_back(property(key,value));

   Messenger_ptr->sendTextMessage(command,properties);
}

// ---------------------------------------------------------------------
// Member function issue_delete_edge_message takes in *mover1_ptr and
// *mover2_ptr and generates an ActiveMQ "DELETE_EDGE" message.  The two
// movers types and IDs are encoded as key-value string pair
// properties.

void movers_group::issue_delete_edge_message(
   mover* mover1_ptr,mover* mover2_ptr)
{
//   cout << "inside movers_group::issue_delete_edge_message()" << endl;
   
   if (Messenger_ptr==NULL ||
       !Messenger_ptr->connected_to_broker_flag()) return;

   string command,key,value;
   typedef pair<string,string> property;
   vector<property> properties;

   command="DELETE_EDGE";

   key="TYPE1";
   value=get_MoverType_string(mover1_ptr);
/*
   if (mover1_ptr->get_MoverType()==mover::ROI)
   {
      value="ROI";
   }
   else if (mover1_ptr->get_MoverType()==mover::VEHICLE)
   {
      value="VEHICLE";
   }
*/
   properties.push_back(property(key,value));

   key="ID1";
   value=stringfunc::number_to_string(mover1_ptr->get_ID());
   properties.push_back(property(key,value));

   key="TYPE2";
   value=get_MoverType_string(mover2_ptr);
/*
   if (mover2_ptr->get_MoverType()==mover::ROI)
   {
      value="ROI";
   }
   else if (mover2_ptr->get_MoverType()==mover::VEHICLE)
   {
      value="VEHICLE";
   }
   else if (mover2_ptr->get_MoverType()==mover::KOZ)
   {
      value="KOZ";
   }
*/
   properties.push_back(property(key,value));

   key="ID2";
   value=stringfunc::number_to_string(mover2_ptr->get_ID());
   properties.push_back(property(key,value));

   Messenger_ptr->sendTextMessage(command,properties);
}

// ---------------------------------------------------------------------
// Member function issue_delete_all_message should be called when a
// map-client user wants to purge all Regions-of-Interest and vehicle
// tracks.  Michael Yee's social network program can then simply
// respond by destroying all vertices and edges.

void movers_group::issue_delete_all_message()
{
//   cout << "inside movers_group::issue_delete_all_message()" << endl;

   if (Messenger_ptr==NULL ||
       !Messenger_ptr->connected_to_broker_flag()) return;

   string command="DELETE_ALL";
   Messenger_ptr->sendTextMessage(command);
}

// ---------------------------------------------------------------------
// Member function issue_add_track_message extracts track positions
// corresponding to distinct direction vectors from input *mover_ptr.
// It converts the position coordinates from UTM eastings and
// northings into longitudes and latitudes.  This method subsequently
// broadcasts a series of (longitude,latitude,altitude) string triples
// from which the track can be reconstructed.

void movers_group::issue_add_track_message(
   bool northern_hemisphere_flag,int UTM_zone,track* track_ptr,
   bool compute_posns_with_distinct_dirs_flag)
{
//   cout << "inside movers_group::issue_add_track_message()" << endl;
//   cout << "*track_ptr = " << *track_ptr << endl;
//   cout << "northern flag = " << northern_hemisphere_flag
//        << " UTM_zone = " << UTM_zone << endl;
   if (Messenger_ptr==NULL) 
   {
      cout << "Messenger_ptr=NULL in movers_group::issue_add_track_message()!"
           << endl;
      return;
   }

   if (!Messenger_ptr->connected_to_broker_flag())
   {
      cout << "Messenger not connected to broker!" << endl;
      return;
   }
   
   vector<threevector> track_posns,track_velocities;
//   cout << "compute_posns_with_distinct_dirs_flag = "
//        << compute_posns_with_distinct_dirs_flag << endl;
   if (compute_posns_with_distinct_dirs_flag)
   {
      track_posns=track_ptr->compute_posns_with_distinct_directions();
   }
   else
   {
      track_posns=track_ptr->get_posns();
   }
   track_velocities=track_ptr->get_velocities();
   unsigned int n_points=track_posns.size();
   unsigned int n_velocities=track_velocities.size();
//   cout << "n points = " << n_points << endl;
//   cout << "n_velocities = " << n_velocities << endl;

   Clock clock;
   vector<double> worldtime,longitude,latitude,altitude;
   for (unsigned int n=0; n<n_points; n++)
   {
      geopoint curr_point(
         northern_hemisphere_flag,UTM_zone,
         track_posns[n].get(0),track_posns[n].get(1),track_posns[n].get(2));
                       
      worldtime.push_back(track_ptr->get_time(n));	// secs since 1/1/1970
      longitude.push_back(curr_point.get_longitude());
      latitude.push_back(curr_point.get_latitude());
      altitude.push_back(curr_point.get_altitude());

//      cout.precision(10);
//      cout << "n = " << n 
//           << " x = " << track_posns[n].get(0)
//           << " y = " << track_posns[n].get(1)
//           << " lng = " << longitude.back()
//           << " lt = " << latitude.back() 
//           << endl;
//      cout << "n = " << n 
//           << " t = " << worldtime.back()
//           << " long = " << longitude.back()
//           << " lat = " << latitude.back() << endl;
      clock.convert_elapsed_secs_to_date(worldtime.back());
      cout << clock.YYYY_MM_DD_H_M_S() << endl;
   } // loop over index n labeling track points

   string command,key,value;
   typedef pair<string,string> property;
   vector<property> properties;

   command="ADD_TRACK";

   key="ID";
   value=stringfunc::number_to_string(track_ptr->get_label_ID());
   properties.push_back(property(key,value));

   key="N_POINTS";
   value=stringfunc::number_to_string(n_points);
   properties.push_back(property(key,value));

   colorfunc::RGB track_RGB=track_ptr->get_RGB_color();
   key="RGB COLOR";
   value=stringfunc::number_to_string(track_RGB.first)+" ";
   value += stringfunc::number_to_string(track_RGB.second)+" ";
   value += stringfunc::number_to_string(track_RGB.third);
   properties.push_back(property(key,value));

   int n_digits=basic_math::mytruncate(log10(n_points))+1;
   for (unsigned int n=0; n<n_points; n++)
   {
      key="TIME LONGITUDE LATITUDE ALTITUDE "
         +stringfunc::integer_to_string(n,n_digits);
      if (n_points==n_velocities)
      {
         key="TIME LONGITUDE LATITUDE ALTITUDE VX VY VZ "
            +stringfunc::integer_to_string(n,n_digits);
      }

// Pad key index n with leading zeros so that ActiveMQ's alphabetical
// ordering of its messages coincides with numerical ordering of track
// waypoints:

      value = stringfunc::number_to_string(worldtime[n])+" ";
      value += stringfunc::number_to_string(longitude[n])+" ";
      value += stringfunc::number_to_string(latitude[n])+" ";
      value += stringfunc::number_to_string(altitude[n])+" ";

      if (n_points==n_velocities)
      {
         threevector curr_V=track_velocities[n];
         value += stringfunc::number_to_string(curr_V.get(0))+" ";
         value += stringfunc::number_to_string(curr_V.get(1))+" ";
         value += stringfunc::number_to_string(curr_V.get(2));
      }

//      cout << "n = " << n
//           << " value = " << value << endl;
      properties.push_back(property(key,value));
   }

   Messenger_ptr->sendTextMessage(command,properties);
}

// ---------------------------------------------------------------------
// Member function issue_delete_track_message

void movers_group::issue_delete_track_message(int track_ID)
{
//   cout << "inside movers_group::issue_delete_track_message()" << endl;

   if (Messenger_ptr==NULL) 
   {
      cout << "Messenger_ptr=NULL in movers_group::issue_delete_track_message() !" << endl;
      return;
   }

   if (!Messenger_ptr->connected_to_broker_flag()) return;

   string command,key,value;
   typedef pair<string,string> property;
   vector<property> properties;

   command="DELETE_TRACK";

   key="ID";
   value=stringfunc::number_to_string(track_ID);
   properties.push_back(property(key,value));

   Messenger_ptr->sendTextMessage(command,properties);
}

