// ==========================================================================
// Program COLLAB_FILTER implements our solution to homework problem
// #2 for the Domingos online course.  See "Empirical Analysis of
// Predictive Algorithms for Collaborative Filtering" by Breese,
// Heckerman and Kadie, 1998.

// As a sanity check, we compute the mean average and RMS errors after
// simply assigning a user's average number of votes to movies which
// he/she never ranked.  The resulting error metrics seem to be only a
// few percentage points worse than those which come from
// collaborative filtering...


// n_training_samples = 3,255,352
// n_training_movies = 1,821
// n_training_users = 28,978
// fill frac = n_movies * n_users / n_training_samples = 6.1%

// User 481 (a = 3) and user 769 (i = 4) have correlation w_ai = 0.167;
// User 481 has votes for movie 111 and 636, but not 156
// User 769 has votes for movie 156 and 636, but not 111

//			       collab_filter

// ==========================================================================
// Last updated on 9/8/15; 9/9/15; 9/10/15; 9/11/15
// ==========================================================================

#include  <algorithm>
#include  <fstream>
#include  <iostream>
#include  <map>
#include  <set>
#include  <string>
#include  <vector>

#include "math/basic_math.h"
#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "time/timefuncs.h"

#include "osg/Custom3DManipulator.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgAnnotators/GraphNodesGroup.h"
#include "osg/osgAnnotators/GraphNodesKeyHandler.h"
#include "osg/ModeController.h"
#include "osg/ModeKeyHandler.h"
#include "osg/osgOperations/Operations.h"
#include "osg/osgWindow/ViewerManager.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::ios;
using std::map;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
   cout.precision(8);
   timefunc::initialize_timeofday_clock();

   typedef map<pair<int, int>, double > MOVIE_USER_VOTES_MAP;
// independent pair var: (movie_ID, user_ID)
// dependent double: number of user votes for movie
   MOVIE_USER_VOTES_MAP training_movie_user_votes_map,
      testing_movie_user_votes_map;

   typedef map<int, vector<pair<int, double> > > USER_MOVIE_VOTES_MAP;
// independent int var: user_ID
// dependent var: vector<(movie_ID, votes)>
   USER_MOVIE_VOTES_MAP training_user_movie_votes_map,
      testing_user_movie_votes_map;

   typedef map<int, int> MOVIE_IDS_MAP;
// independent int var: movie_ID
// dependent int: consecutive movie index
   MOVIE_IDS_MAP training_movie_ids_map;

   typedef map<int, int> USER_IDS_MAP;
// independent int var: user_ID
// dependent int: consecutive user index
   USER_IDS_MAP training_user_ids_map;

   typedef map<int, double > USER_AVG_VOTES_MAP;
// independent int var: user_ID
// dependent pair: average number of votes cast by user
   USER_AVG_VOTES_MAP user_avg_votes_map;

   TABLE_LABELS_MAP table_labels_map;

// Import movie/user/votes training data into various hash maps:

   string banner="Importing movie/user/votes training data:";
   outputfunc::write_banner(banner);

   string training_filename="./data/TrainingRatings.txt";
   filefunc::ReadInfile(training_filename);
   int n_training_samples = filefunc::text_line.size();
   cout << "n_training_samples = " << n_training_samples << endl;
   string separator_chars = ",";
   int user_index = 0, movie_index = 0;
   for(int i=0; i < n_training_samples; i++)
   {
      outputfunc::update_progress_fraction(i,200000,n_training_samples);

      vector<double> values = stringfunc::string_to_numbers(
         filefunc::text_line[i], separator_chars);
      int movie_ID = values[0];
      int user_ID = values[1];
      double votes = values[2];

      pair<int,int> P1;
      P1.first = movie_ID;
      P1.second = user_ID;
      training_movie_user_votes_map[P1] = votes;

      pair<int,double> P2;
      P2.first = movie_ID;
      P2.second = votes;

      USER_MOVIE_VOTES_MAP::iterator iter = training_user_movie_votes_map.
         find(user_ID);
      if(iter == training_user_movie_votes_map.end())
      {
         vector< pair<int,double> > V;
         V.push_back(P2);
         training_user_movie_votes_map[user_ID] = V;
      }
      else
      {
         iter->second.push_back(P2);
      }

      USER_IDS_MAP::iterator iter2 = training_user_ids_map.find(user_ID);
      if(iter2 == training_user_ids_map.end())
      {
         training_user_ids_map[user_ID] = user_index;
         user_index++;
      }

      MOVIE_IDS_MAP::iterator iter3 = training_movie_ids_map.find(movie_ID);
      if(iter3 == training_movie_ids_map.end())
      {
         training_movie_ids_map[movie_ID] = movie_index;
         movie_index++;
      }

   } // loop over index i labeling training samples
   cout << endl;

   cout << "training_movie_user_votes_map.size() = " 
        << training_movie_user_votes_map.size() << endl;
   cout << "training_user_movie_votes_map.size() = " 
        << training_user_movie_votes_map.size() << endl;

// Consolidate training user IDs within sorted STL vector.  Save
// consecutive indices for training user IDs within STL map.

   vector<int> training_user_IDs;
   for(USER_IDS_MAP::iterator iter = training_user_ids_map.
          begin(); iter != training_user_ids_map.end(); iter++)
   {
      int user_ID = iter->first;
      training_user_IDs.push_back(user_ID);
   }

   std::sort(training_user_IDs.begin(), training_user_IDs.end());
   int n_training_users = training_user_IDs.size();
   cout << "n_training_users = " << n_training_users << endl;

   training_user_ids_map.clear();
   for(int i = 0; i < n_training_users; i++)
   {
      int user_ID = training_user_IDs[i];
      training_user_ids_map[user_ID] = i;
   }
   
// Consolidate training movie IDs within sorted STL vector.  Save
// consecutive indices for training movie IDs within STL map.

   vector<int> training_movie_IDs;
   for(MOVIE_IDS_MAP::iterator iter = training_movie_ids_map.begin(); 
       iter != training_movie_ids_map.end(); iter++)
   {
      int movie_ID = iter->first;
      training_movie_IDs.push_back(movie_ID);
   }

   std::sort(training_movie_IDs.begin(), training_movie_IDs.end());
   int n_training_movies = training_movie_IDs.size();
   cout << "n_training_movies = " << n_training_movies << endl;

   training_movie_ids_map.clear();
   for(int i = 0; i < n_training_movies; i++)
   {
      int movie_ID = training_movie_IDs[i];
      training_movie_ids_map[movie_ID] = i;
   }

// For visualization purposes, we do NOT work with raw movie and user
// IDs, for their number is sparse!  Instead, we exchange original
// movie and user IDs for contiguous indices that range from 0 to
// n_training_movies-1 and 0 to n_training_users-1.  Only then can we
// start to visualize an (incomplete) 2D table of movie ID vs user ID:

   for(MOVIE_USER_VOTES_MAP::iterator iter = 
          training_movie_user_votes_map.begin(); 
       iter != training_movie_user_votes_map.end(); iter++)
   {
      int movie_ID = iter->first.first;
      int user_ID = iter->first.second;
      double votes = iter->second;

      string curr_label;
      vector<string> labels;
      curr_label = "movie_ID: "+stringfunc::number_to_string(movie_ID);
      labels.push_back(curr_label);
      curr_label = "user_ID: "+stringfunc::number_to_string(user_ID);
      labels.push_back(curr_label);
      curr_label = "votes: "+stringfunc::number_to_string(votes);
      labels.push_back(curr_label);

      MOVIE_IDS_MAP::iterator iter2 = training_movie_ids_map.find(movie_ID);
      int movie_index_for_ID = iter2->second;
      USER_IDS_MAP::iterator iter3 = training_user_ids_map.find(user_ID);
      int user_index_for_ID = iter3->second;

      pair<int,int> P3;
      P3.first = movie_index_for_ID;
      P3.second = user_index_for_ID;
      table_labels_map[P3] = labels;
   }

// Import movie/user/votes testing data:

   banner="Importing movie/user/votes testing data:";
   outputfunc::write_banner(banner);

   string testing_filename="./data/TestingRatings.txt";
   filefunc::ReadInfile(testing_filename);
   int n_testing_samples = filefunc::text_line.size();
   cout << "n_testing_samples = " << n_testing_samples << endl;
   for(int i=0; i < n_testing_samples; i++)
   {
      vector<double> values = stringfunc::string_to_numbers(
         filefunc::text_line[i], separator_chars);
      int movie_ID = values[0];
      int user_ID = values[1];
      double votes = values[2];

      pair<int,int> P1;
      P1.first = movie_ID;
      P1.second = user_ID;
      testing_movie_user_votes_map[P1] = votes;

      pair<int,double> P2;
      P2.first = movie_ID;
      P2.second = votes;

      USER_MOVIE_VOTES_MAP::iterator iter = testing_user_movie_votes_map.
         find(user_ID);
      if(iter == testing_user_movie_votes_map.end())
      {
         vector< pair<int,double> > V;
         V.push_back(P2);
         testing_user_movie_votes_map[user_ID] = V;
      }
      else
      {
         iter->second.push_back(P2);
      }
   } // loop over index i labeling testing samples
   cout << endl;

   cout << "testing_movie_user_votes_map.size() = " 
        << testing_movie_user_votes_map.size() << endl;
   cout << "testing_user_movie_votes_map.size() = " 
        << testing_user_movie_votes_map.size() << endl;

// Compute and store average number of votes cast by each user:

   banner="Computing average number of votes cast by each user:";
   outputfunc::write_banner(banner);
   for(USER_MOVIE_VOTES_MAP::iterator iter = training_user_movie_votes_map.
          begin(); iter != training_user_movie_votes_map.end(); iter++)
   {
      int user_ID = iter->first;
      vector<pair<int, double> > V = iter->second;

      double avg_votes = 0;
      if(V.size() == 0) 
      {
         cout << "V.size() == 0 !!! " << endl;
         exit(-1);
      }
      
      for(unsigned int v = 0; v < V.size(); v++)
      {
         pair<int, double> P = V[v];
         avg_votes += P.second;
      }
      avg_votes /= V.size();
      user_avg_votes_map[user_ID] = avg_votes;
//      cout << "user_ID = " << user_ID << " avg_votes = " << avg_votes
//           << endl;
   } // loop over iterator labeling users within training_user_movie_votes_map

// Subtract off mean votes for each user:

   banner="Renormalizing user votes:";
   outputfunc::write_banner(banner);

   for(USER_MOVIE_VOTES_MAP::iterator iter_a = training_user_movie_votes_map.
          begin(); iter_a != training_user_movie_votes_map.end(); iter_a++)
   {
      int user_a_ID = iter_a->first;
      vector<pair<int, double> > V_a = iter_a->second;	// (movie ID, vote)
      double avg_votes_a = user_avg_votes_map[user_a_ID];

      for(unsigned int j = 0; j < V_a.size(); j++)
      {
         double renorm_votes_aj = V_a[j].second - avg_votes_a;   
         iter_a->second.at(j).second = renorm_votes_aj;
      }
   } // loop over iter_a labeling active users

   banner="Comparing predicted vs actual movie votes for users";
   outputfunc::write_banner(banner);
   int counter = 0;

   int n_measurements = 0;
   double mean_absolute_error = 0, mean_absolute_error_2 = 0;
   double mean_square_error = 0, mean_square_error_2 = 0;

   string predictions_filename="./data/movie_predictions.dat";
   ofstream outstream;
   filefunc::openfile(predictions_filename, outstream);
   outstream << "# N_measurements   User_a_ID   Movie_j_ID   Actual votes   Predicted votes  Mean_abs_error  RMS_error" 
             << endl << endl;

   for(unsigned int a = 0; a < training_user_IDs.size(); a++)
   {
      int user_a_ID = training_user_IDs[a];

      double progress_frac = outputfunc::update_progress_fraction(
         counter, 10, training_user_IDs.size());
      if(counter%100==0)
      {
         cout << endl;
         outputfunc::print_elapsed_and_remaining_time(progress_frac);
      }
      counter++;

      USER_MOVIE_VOTES_MAP::iterator iter_a = training_user_movie_votes_map.
         find(user_a_ID);
      vector<pair<int, double> > V_a = iter_a->second;// (movie ID,renorm vote)

      pair<int, double> P3;
      vector<pair<int, double> > user_correlation_V; // (user ID,correl)

      for(unsigned int i = 0; i < training_user_IDs.size(); i++)
      {
         int user_i_ID = training_user_IDs[i];
         if(user_i_ID <= user_a_ID) continue;
      
         USER_MOVIE_VOTES_MAP::iterator iter_i = 
            training_user_movie_votes_map.find(user_i_ID);
         vector<pair<int, double> > V_i = iter_i->second;

         double numer_term = 0;
         double denom_term1 = 0;
         double denom_term2 = 0;
         for(unsigned int j = 0; j < V_a.size(); j++)
         {
            for(unsigned int k = 0; k < V_i.size(); k++)
            {
// Check if movie IDs for user a and user i match:

               if (V_a[j].first != V_i[k].first) continue;

               double renorm_votes_aj = V_a[j].second;
               double renorm_votes_ij = V_i[k].second;
               numer_term +=  renorm_votes_aj * renorm_votes_ij;
               denom_term1 += sqr(renorm_votes_aj);
               denom_term2 += sqr(renorm_votes_ij);
            } // loop over index k 
         } // loop over index j 

         double w_ai = 0;
         const double TINY = 1E-6;
         if(fabs(denom_term1) > TINY && fabs(denom_term2) > TINY)
         {
            w_ai = numer_term / sqrt(denom_term1 * denom_term2);
         }
//         cout << "a = " << a << " user_a_ID = " << user_a_ID << endl;
//         cout << "i = " << i << " user_i_ID = " << user_i_ID << endl;
//         cout << "w_ai = " << w_ai << endl;

         P3.first = user_i_ID;
         P3.second = w_ai;
         user_correlation_V.push_back(P3);

      } // loop over index i labeling users other than current active user

/*
// Compare actual vs predicted votes for training set users:

      const double SMALL = 1E-4;
      double abs_weight_sum = 0;
      double avg_votes_a = user_avg_votes_map[user_a_ID];

      USER_MOVIE_VOTES_MAP::iterator train_iter_a = 
         training_user_movie_votes_map.find(user_a_ID);
      if(train_iter_a != training_user_movie_votes_map.end())
      {
         vector<pair<int, double> > V_a_train = train_iter_a->second; 
         // (movie ID, renorm vote)

         for(unsigned int j = 0; j < V_a_train.size(); j++)
         {
            int movie_j_ID = V_a_train[j].first;
            double sum = 0;

            for(unsigned int i = 0; i < user_correlation_V.size(); i++)
            {
               int user_i_ID = user_correlation_V[i].first;
               double w_ai = user_correlation_V[i].second;

               if(fabs(w_ai) < SMALL) continue;
               abs_weight_sum += fabs(w_ai);

               USER_MOVIE_VOTES_MAP::iterator iter_i = 
                  training_user_movie_votes_map.find(user_i_ID);
               vector<pair<int, double> > V_i = iter_i->second; 
               // (movie ID, renorm vote)

// Check whether user_i voted for movie_j:

               for(unsigned int k = 0; k < V_i.size(); k++)
               {
                  if(movie_j_ID == V_i[k].first)
                  {
                     sum += w_ai * V_i[k].second;
                     break;
                  }
               } // loop over index k 
            } // loop over index i 

            double actual_train_votes = V_a_train[j].second + avg_votes_a;
            double predicted_train_votes = -1;
            if(abs_weight_sum > 0)
            {
               predicted_train_votes = avg_votes_a + sum / abs_weight_sum;
            }

            cout << user_a_ID << "   "
                 << movie_j_ID << "    "
                 << actual_train_votes << "    "
                 << predicted_train_votes << endl;

         } // loop over index j labeling movie IDs for user_a within test set
      } // test_iter_a != testing_user_movie_votes_map.end() conditional
*/

// Compare actual vs predicted votes for testing set users:

      const double SMALL = 1E-4;
      double abs_weight_sum = 0;
      double avg_votes_a = user_avg_votes_map[user_a_ID];
//      double default_votes = 2;	// BAD GUESS
//      double default_votes = 2.8;	// POOR GUESS
//      double default_votes = 3;	// OK GUESS
//       double default_votes = 3.2;	// BETTER GUESS
//      double default_votes = 3.4;	// SIMILAR to 3.2 GUESS

      USER_MOVIE_VOTES_MAP::iterator test_iter_a = 
         testing_user_movie_votes_map.find(user_a_ID);
      if(test_iter_a != testing_user_movie_votes_map.end())
      {
         vector<pair<int, double> > V_a_test = test_iter_a->second; 
         // (movie ID, renorm vote)

         for(unsigned int j = 0; j < V_a_test.size(); j++)
         {
            int movie_j_ID = V_a_test[j].first;
            double sum = 0;
            for(unsigned int i = 0; i < user_correlation_V.size(); i++)
            {
               int user_i_ID = user_correlation_V[i].first;
               double w_ai = user_correlation_V[i].second;

               if(fabs(w_ai) < SMALL) continue;
               abs_weight_sum += fabs(w_ai);

               USER_MOVIE_VOTES_MAP::iterator iter_i = 
                  training_user_movie_votes_map.find(user_i_ID);
               vector<pair<int, double> > V_i = iter_i->second; 
               // (movie ID, renorm vote)

// Check whether user_i voted for movie_j:

	       bool user_i_voted_for_movie_j_flag = false;
               for(unsigned int k = 0; 
                   k < V_i.size() && !user_i_voted_for_movie_j_flag; k++)
               {
                  if(movie_j_ID == V_i[k].first)
                  {
                     sum += w_ai * V_i[k].second;
                     user_i_voted_for_movie_j_flag = true;
                  }
               }

// If user_i did NOT vote for movie_j, we can assume user_i cast some
// default number of votes.  But for at least small numbers of
// experiments which we performed on 9/11/15, default vote assigning
// does not appear to yield significant improvements in mean error
// results...

/*
               if(!user_i_voted_for_movie_j_flag)
               {
                  sum += w_ai * (default_votes - user_avg_votes_map[user_i_ID]);
               }
*/
             
            } // loop over index i 

            double actual_test_votes = V_a_test[j].second;
            double predicted_test_votes = -1;
            if(abs_weight_sum > 0)
            {
               predicted_test_votes = avg_votes_a + sum / abs_weight_sum;
            }

            double delta = actual_test_votes - predicted_test_votes;
	    double delta_2 = actual_test_votes - avg_votes_a;
            mean_absolute_error = (n_measurements * mean_absolute_error + 
                                   fabs(delta) ) / (n_measurements + 1);
            mean_absolute_error_2 = (n_measurements * mean_absolute_error_2 + 
                                   fabs(delta_2) ) / (n_measurements + 1);
            mean_square_error = 
               (n_measurements * mean_square_error + sqr(delta)) / 
               (n_measurements + 1);
            mean_square_error_2 = 
               (n_measurements * mean_square_error_2 + sqr(delta_2)) / 
               (n_measurements + 1);
            double rms_error = sqrt(mean_square_error);
            double rms_error_2 = sqrt(mean_square_error_2);
            n_measurements++;

            cout << n_measurements << "     "
                 << mean_absolute_error << "     "
                 << mean_absolute_error_2 << "     "
                 << rms_error << "     "
                 << rms_error_2 
                 << endl;
            
            outstream << n_measurements << "    "
                      << user_a_ID << "   "
                      << movie_j_ID << "     "
                      << actual_test_votes << "     "
                      << predicted_test_votes << "     "
                      << mean_absolute_error << "     "
                      << rms_error 
                      << endl;

         } // loop over index j labeling movie IDs for user_a within test set
      } // test_iter_a != testing_user_movie_votes_map.end() conditional


   } // loop over index a labeling active user IDs

   filefunc::closefile(predictions_filename, outstream);


   exit(-1);
   
// ---------------------------------------------------------
// Display table using our OSG GraphNodesGroup class:

   const int ndims=3;

// Construct the viewer and instantiate ViewerManager:

   WindowManager* window_mgr_scenegraph_ptr=new ViewerManager();
   window_mgr_scenegraph_ptr->initialize_window("Table");
   osg::Group* root = new osg::Group;

// Instantiate Operations object to handle mode, animation and image
// number control:

   Operations operations(ndims,window_mgr_scenegraph_ptr);
   ModeController* ModeController_ptr=operations.get_ModeController_ptr();
   ModeController_ptr->setState(ModeController::MANIPULATE_GRAPHNODE);

// Add custom manipulators:

   bool disable_rotations_flag=true;
   osgGA::Custom3DManipulator* CM_scenegraph_ptr=
      new osgGA::Custom3DManipulator(
      ModeController_ptr,window_mgr_scenegraph_ptr,disable_rotations_flag);
   window_mgr_scenegraph_ptr->set_CameraManipulator(CM_scenegraph_ptr);

// Instantiate group to hold all decorations:

   Decorations decorations(
      window_mgr_scenegraph_ptr,ModeController_ptr,CM_scenegraph_ptr);

// Insert scenegraph display's grid:

   double min_X=0;
   double max_X=10;
   double min_Y=0;
   double max_Y=10;
   double min_Z=0;
   bool world_origin_precisely_in_lower_left_corner=true;

   AlirtGrid* scenegraph_grid_ptr=decorations.add_AlirtGrid(
      ndims, NULL, min_X, max_X, min_Y, max_Y, min_Z,
      world_origin_precisely_in_lower_left_corner);
   threevector* scenegraph_grid_origin_ptr=
      scenegraph_grid_ptr->get_world_origin_ptr();

   scenegraph_grid_ptr->set_axes_labels("X","Y");
   scenegraph_grid_ptr->set_delta_xy(2,2);
   scenegraph_grid_ptr->set_axis_char_label_size(1);
   scenegraph_grid_ptr->set_tick_char_label_size(1);
   scenegraph_grid_ptr->update_grid();

// Instantiate a GraphNodes group:

   GraphNodesGroup graphnodes_group(NULL,scenegraph_grid_origin_ptr);
   graphnodes_group.set_display_scenegraph_flag(false);

   root->addChild(graphnodes_group.createBoxLight(threevector(20,10,10)));
   root->addChild(graphnodes_group.get_OSGgroup_ptr());

// Instantiate a GraphNodesKeyHandler for debugging purposes:

   window_mgr_scenegraph_ptr->get_EventHandlers_ptr()->push_back(
      new GraphNodesKeyHandler(&graphnodes_group,ModeController_ptr));

   graphnodes_group.set_table_labels_map_ptr(&table_labels_map);
   graphnodes_group.generate_Graph_from_table(
      250, 500);
//      500, 1000);
//      n_training_movies/10, n_training_users/100);
//      n_training_movies, n_training_users);

// Optimize the scene graph, remove redundent nodes and states, and
// then attach it to the viewer:

   window_mgr_scenegraph_ptr->setSceneData(root);

// Create the windows and run the threads:

   window_mgr_scenegraph_ptr->realize();

// Set initial camera lateral posn to grid's midpoint and scale its
// altitude according to grid's maximal linear dimension:

   CM_scenegraph_ptr->set_eye_to_center_distance(
      basic_math::max(
         scenegraph_grid_ptr->get_xsize(),scenegraph_grid_ptr->get_ysize()));
   CM_scenegraph_ptr->update_M_and_Minv();

   while( !window_mgr_scenegraph_ptr->done() )
   {
      window_mgr_scenegraph_ptr->process();
   }

   delete window_mgr_scenegraph_ptr;
}

