// ==========================================================================
// CONTAINERFUNCS stand-alone methods
// ==========================================================================
// Last modified on 7/8/05
// ==========================================================================

#include "math/basic_math.h"
#include "math/constants.h"
#include "datastructures/containerfuncs.h"
#include "datastructures/Linkedlist.h"
#include "plot/metafile.h"
#include "datastructures/Mynode.h"
#include "templates/mytemplates.h"

using std::cout;
using std::endl;
using std::ostream;
using std::string;

namespace containerfunc
{

// ==========================================================================
// Linked list methods:
// ==========================================================================

// Member function scale_variable_value traverses through the list
// and multiplies each node's independent variable var[varnumber] by
// input parameter a:
      
   void scale_variable_values(linkedlist* list_ptr,int varnumber,double a)
      {
         mynode *currnode_ptr=list_ptr->get_start_ptr();
         while (currnode_ptr != NULL)
         {
            currnode_ptr->get_data().set_var(
               varnumber,currnode_ptr->get_data().get_var(varnumber)*a);
            currnode_ptr=currnode_ptr->get_nextptr();
         }
      }

// ---------------------------------------------------------------------
// Member function scale_node_func_value traverses through the list
// and multiplies each node's function value by input parameter a:
      
   void scale_node_func_values(linkedlist* list_ptr,double a)
      {
         mynode *currnode_ptr=list_ptr->get_start_ptr();
         while (currnode_ptr != NULL)
         {
            currnode_ptr->get_data().set_func(
               0,currnode_ptr->get_data().get_func(0)*a);
            currnode_ptr=currnode_ptr->get_nextptr();
         }
      }

// ---------------------------------------------------------------------
// Member function find_max_min_vars traverses through the list and
// saves the maximimum/minimum of var[0] within member variables
// xmax/xmin.

/*
   void find_max_min_vars(linkedlist* list_ptr)
      {
         list_ptr->get_metafile_ptr()->
            set_xbounds(POSITIVEINFINITY,NEGATIVEINFINITY);
         mynode *currnode_ptr=list_ptr->get_start_ptr();
         while (currnode_ptr != NULL)
         {
            double curr_var=currnode_ptr->get_data().get_var(0);
            list_ptr->get_metafile_ptr()->set_xbounds(
               min(curr_var,list_ptr->get_metafile_ptr()->get_xmin()),
               max(curr_var,list_ptr->get_metafile_ptr()->get_xmax()));
            currnode_ptr=currnode_ptr->get_nextptr();
         }
      }
*/

/*
// ---------------------------------------------------------------------
// Member function find_max_min_func_values traverses through the list
// and saves the maximimum/minimum of all n_depend_vars function
// values within member variables fmax/fmin.

   void find_max_min_func_values(linkedlist* list_ptr)
      {
         find_max_min_func_values(list_ptr,NEGATIVEINFINITY,POSITIVEINFINITY);
      }

   void find_max_min_func_values(
      linkedlist* list_ptr,double var_lo,double var_hi)
      {
         double global_fmin=POSITIVEINFINITY;
         double global_fmax=NEGATIVEINFINITY;
         for (int n=0; n<list_ptr->get_start_ptr()->get_data().
                 get_n_depend_vars(); n++)
         {
            find_max_min_func_values(list_ptr,n,var_lo,var_hi);
            global_fmin=min(global_fmin,list_ptr->get_fmin());
            global_fmax=max(global_fmax,list_ptr->get_fmax());
         }
         list_ptr->set_fmin(global_fmin);
         list_ptr->set_fmax(global_fmax);
      }

// This overloaded version of find_max_min_func_values returns the
// extremal values of the dependent variables labeled by input integer
// func_number:

   void find_max_min_func_values(linkedlist* list_ptr,int func_number)
      {
         find_max_min_func_values(
            list_ptr,func_number,NEGATIVEINFINITY,POSITIVEINFINITY);
      }

   void find_max_min_func_values(
      linkedlist* list_ptr,int func_number,double var_lo,double var_hi)
      {
         if (func_number < list_ptr->get_start_ptr()->get_data().
             get_n_depend_vars() && func_number >= 0)
         {
            int n_nodes=list_ptr->size();
            double var_maximum[n_nodes];
            double var_minimum[n_nodes];
            find_max_min_func_values(
               list_ptr,func_number,var_lo,var_hi,var_maximum,var_minimum);
         }
         else
         {
            cout << "Error inside containerfunc::find_max_min_func_values()" 
                 <<endl;
            cout << "func_number = " << func_number << " is greater than "
                 << " n_depend_vars = " 
                 << list_ptr->get_start_ptr()->get_data().
               get_n_depend_vars() << endl;
         }
      }

   void find_max_min_func_values(
      linkedlist* list_ptr,double var_maximum[],double var_minimum[])
      {
         find_max_min_func_values(list_ptr,0,var_maximum,var_minimum);
      }

// In this overloaded version of find_max_min_func_values, the
// extremal nodes' independent variables are also returned in
// var_maximum and var_minimum.

   void find_max_min_func_values(
      linkedlist* list_ptr,int func_number,
      double var_maximum[],double var_minimum[])
      {
         find_max_min_func_values(
            list_ptr,func_number,NEGATIVEINFINITY,POSITIVEINFINITY,
            var_maximum,var_minimum);
      }

   void find_max_min_func_values(
      linkedlist* list_ptr,int func_number,double var_lo,double var_hi,
      double var_maximum[],double var_minimum[])
      {
         list_ptr->set_fmax(NEGATIVEINFINITY);
         list_ptr->set_fmin(POSITIVEINFINITY);
         mynode *currnode_ptr=list_ptr->get_start_ptr();

         while (currnode_ptr != NULL)
         {
            double curr_var=currnode_ptr->get_data().get_var(0);
            if (curr_var > var_lo && curr_var < var_hi)
            {
               double currf=currnode_ptr->get_data().get_func(func_number);
               if (currf < list_ptr->get_fmin())
               {
                  list_ptr->set_fmin(currf);
                  for (int i=0; i<currnode_ptr->get_data().
                          get_n_indep_vars(); i++)
                  {
                     var_minimum[i]=currnode_ptr->get_data().get_var(i);
                  }
               }
               if (currf > list_ptr->get_fmax())
               {
                  list_ptr->set_fmax(currf);
                  for (int i=0; i<currnode_ptr->get_data().
                          get_n_indep_vars(); i++)
                  {
                     var_maximum[i]=currnode_ptr->get_data().get_var(i);
                  }
               }
            } // curr_var > var_lo && curr_var < var_hi conditional
            currnode_ptr=currnode_ptr->get_nextptr();
         }
      }

// This next method assumes that the list contains central means plus
// error bars stored in its zeroth and first dependent function
// values.  It scans through the list to determine the maximum value
// of mean+error and minimum value of mean-error.  This method is
// helpful for plotting purposes where we don't want to chop off error
// bars...

   void find_max_min_values_of_func_plus_errors(linkedlist* list_ptr)
      {
         list_ptr->set_fmax(NEGATIVEINFINITY);
         list_ptr->set_fmin(POSITIVEINFINITY);
         mynode *currnode_ptr=list_ptr->get_start_ptr();
         while (currnode_ptr != NULL)
         {
            double currf=currnode_ptr->get_data().get_func(0);
            double curr_error=currnode_ptr->get_data().get_func(1);

            if (currf-curr_error < list_ptr->get_fmin())
            {
               list_ptr->set_fmin(currf-curr_error);
            }
            if (currf+curr_error > list_ptr->get_fmax())
            {
               list_ptr->set_fmax(currf+curr_error);
            }
            currnode_ptr=currnode_ptr->get_nextptr();
         }
      }
*/

// ---------------------------------------------------------------------
// Method sort_nodes_by_indep_var rearranges the nodes
// within the linkedlist so that their independent variables are
// either monotonically decreasing or increasing, depending upon input
// boolean parameter decreasing_func.

   void sort_nodes_by_indep_var(linkedlist* list_ptr,bool decreasing_func)
      {
         int n_nodes=list_ptr->size();
         if (n_nodes >= 2) 
         {
            double var[n_nodes];
            int counter=0;
            mynode* currnode_ptr=list_ptr->get_start_ptr();
            do 
            {
               var[counter++]=currnode_ptr->get_data().get_var(0);
               currnode_ptr=currnode_ptr->get_nextptr();
            } 
            while (currnode_ptr != NULL);
            list_ptr->sort_nodes(var,decreasing_func);
         } // n_nodes >= 2 conditional
      }

// ---------------------------------------------------------------------
// Method sort_nodes_by_depend_var rearranges the nodes
// within the linkedlist so that their d-th dependent variables are
// either monotonically decreasing or increasing, depending upon input
// boolean parameter decreasing_func.

   void sort_nodes_by_depend_var(
      linkedlist* list_ptr,int d,bool decreasing_func)
      {
         int n_nodes=list_ptr->size();
         if (n_nodes >= 2)
         {
            double func[n_nodes];
            int counter=0;
            mynode* currnode_ptr=list_ptr->get_start_ptr();
            do 
            {
               func[counter++]=currnode_ptr->get_data().get_func(d);
               currnode_ptr=currnode_ptr->get_nextptr();
            } 
            while (currnode_ptr != NULL);
            list_ptr->sort_nodes(func,decreasing_func);
         } // n_nodes >= 2 conditional
      }

/*
   void sort_nodes_by_score(
      Linkedlist<satellitetrajectory>* list_ptr,bool decreasing_func)
      {
         int n_nodes=list_ptr->size();
         if (n_nodes >= 2)
         {
            double score[n_nodes];
            int counter=0;
            Mynode<satellitetrajectory>* currnode_ptr=
               list_ptr->get_start_ptr();
            do 
            {
               score[counter++]=currnode_ptr->get_data().get_score();
               currnode_ptr=currnode_ptr->get_nextptr();
            } 
            while (currnode_ptr != NULL);
            list_ptr->sort_nodes(score,decreasing_func);
         } // n_nodes >= 2 conditional
      }
*/

} // containerfunc namespace










