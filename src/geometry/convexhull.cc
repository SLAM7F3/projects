// This ancient class is DEPRECATED.  Use QHULL instead!


// ==========================================================================
// Graham's convex hull algorithm
// ==========================================================================
// Last updated on 8/20/04; 4/13/06; 8/3/06; 8/5/06; 9/23/06
// ==========================================================================

// This code is described in "Computational Geometry in C" (Second
// Edition) by Joseph O'Rourke, Chapter 3.  It is not written to be
// comprehensible without the explanation in that book.

// This code is Copyright 1998 by Joseph O'Rourke.  It may be freely
// redistributed in its entirety provided that this copyright notice
// is not removed.

#include <math.h>
#include <new>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "math/basic_math.h"
#include "geometry/convexhull.h"
#include "datastructures/Linkedlist.h"
#include "datastructures/Mynode.h"
#include "templates/mytemplates.h"
#include "geometry/polygon.h"

using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::ios;
using std::istream;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

namespace convexhull
{
   int npoints = 0;                     // Actual # of points 
   int ndelete = 0;                   	// Number deleted 

   void Initialize_Points(const vector<threevector>& currpoint)
      {
         npoints=currpoint.size();
         for (int n=0; n<npoints; n++)
         {
            P[n].v[0]=currpoint[n].get(0);
            P[n].v[1]=currpoint[n].get(1);
            P[n].vnum=n;
            P[n].delete_flag=false;
         }
      }
 
/*---------------------------------------------------------------------
  FindLowest finds the rightmost lowest point and swaps with 0-th.
  The lowest point has the min y-coord, and amongst those, the
  max x-coord: so it is rightmost among the lowest.
  ---------------------------------------------------------------------*/

   void FindLowest(void)
      {
         int m = 0;   // Index of lowest so far
         for (int i = 1; i < npoints; i++ )
            if ( (P[i].v[Y] <  P[m].v[Y]) ||
                 ((P[i].v[Y] == P[m].v[Y]) && (P[i].v[X] > P[m].v[X])) ) 
               m = i;
//         cout << "Swapping " << m << " with 0" << endl;
         Swap(0,m); // Swap P[0] and P[m] 
      }

   void	Swap(int i,int j)
      {
         templatefunc::swap(P[i].vnum,P[j].vnum);
         templatefunc::swap(P[i].v[X],P[j].v[X]);
         templatefunc::swap(P[i].v[Y],P[j].v[Y]);
         templatefunc::swap(P[i].delete_flag,P[j].delete_flag);
      }

/*---------------------------------------------------------------------*/

   void Sort_Points()
      {
         qsort(&P[1],npoints-1,sizeof(tsPoint),Compare);
      }

/*---------------------------------------------------------------------
  Compare: returns -1,0,+1 if p1 < p2, =, or > respectively;
  here "<" means smaller angle.  Follows the conventions of qsort.
  ---------------------------------------------------------------------*/

   int Compare(const void *tpi_ptr,const void *tpj_ptr)
      {
         tsPoint* pi_ptr = (tsPoint*) tpi_ptr;
         tsPoint* pj_ptr = (tsPoint*) tpj_ptr;

         int a = AreaSign(P[0].v,pi_ptr->v,pj_ptr->v);	// area
         if (a > 0)
            return -1;
         else if (a < 0)
            return 1;
         else 
         { /* Collinear with P[0] */

            // x & y are projections of ri & rj in 1st quadrant 
            double x=fabs(pi_ptr->v[X]-P[0].v[X])-fabs(pj_ptr->v[X]-P[0].v[X]);
            double y=fabs(pi_ptr->v[Y]-P[0].v[Y])-fabs(pj_ptr->v[Y]-P[0].v[Y]);

            ndelete++;
            if ( (x < 0) || (y < 0) ) {
               pi_ptr->delete_flag=true;
               return -1;
            }
            else if ( (x > 0) || (y > 0) ) {
               pj_ptr->delete_flag = true;
               return 1;
            }
            else { /* points are coincident */
               if (pi_ptr->vnum > pj_ptr->vnum)
                  pj_ptr->delete_flag = true;
               else 
                  pi_ptr->delete_flag = true;
               return 0;
            }
         }
      }

/*---------------------------------------------------------------------
  Pops off top element of stack s, frees up the cell, and returns new top.
  ---------------------------------------------------------------------*/

   tsStack* Pop(tsStack* s_ptr)
      {
         tsStack* top_ptr = s_ptr->next_ptr;
         delete s_ptr;
         return top_ptr;
      }


/*---------------------------------------------------------------------
  Get a new cell, fill it with p, and push it onto the stack.
  Return pointer to new stack top.
  ---------------------------------------------------------------------*/

   tsStack* Push(tsPoint* p_ptr,tsStack* top_ptr)
      {
         /* Get new cell and fill it with point. */
         tsStack* s_ptr=new tsStack;
         s_ptr->p_ptr = p_ptr;
         s_ptr->next_ptr = top_ptr;
         return s_ptr;
      }

/*---------------------------------------------------------------------*/
   void PrintStack(tsStack* t_ptr)
      {
         if (t_ptr==NULL) cout << "Empty stack" << endl;
         while (t_ptr != NULL) { 
            cout << "vnum = " << t_ptr->p_ptr->vnum
                 << " x = " << t_ptr->p_ptr->v[X]
                 << " y = " << t_ptr->p_ptr->v[Y] << endl;
            t_ptr=t_ptr->next_ptr;
         }
      }

/*---------------------------------------------------------------------*/
   int Stacklength(tsStack* t_ptr)
      {
         int length=0;
         while (t_ptr != NULL) 
         { 
            length++;
            t_ptr=t_ptr->next_ptr;
         }
         return length;
      }

/*---------------------------------------------------------------------*/
   void delete_stack(tsStack* s_ptr)
      {
         while (s_ptr != NULL) 
         { 
            tsStack* next_ptr=s_ptr->next_ptr;
            delete s_ptr;
            s_ptr=next_ptr;
         }
      }

/*---------------------------------------------------------------------
  Squash removes all elements from P marked delete.
  ---------------------------------------------------------------------*/

   void Squash(void)
      {
         int i = 0;
         int j = 0;
         /*printf("Squash: n=%d\n",n);*/
         while ( i < npoints ) 
         {
            /*printf("i=%d,j=%d\n",i,j);*/
            if ( !P[i].delete_flag) 
            { /* if not marked for deletion */
               Copy( i, j ); /* Copy P[i] to P[j]. */
               j++;
            }
            /* else do nothing: delete by skipping. */
            i++;
         }
         npoints = j;

//         cout << "After Squash: npoints = " << npoints << endl;
//         PrintPoints();
      }

/*---------------------------------------------------------------------
  Performs the Graham scan on an array of angularly sorted points P.
  ---------------------------------------------------------------------*/
   tsStack* Graham()
      {
         /* Initialize stack. */
         tsStack* top_ptr=NULL;
         top_ptr = Push ( &P[0], top_ptr );
         top_ptr = Push ( &P[1], top_ptr );

         /* Bottom two elements will never be removed. */
         int i = 2;

         while ( i < npoints ) 
         {
//            cout << "Stack at top of while loop, i = " << i 
//                 << " vnum = " << P[i].vnum << endl;
//            PrintStack(top_ptr);
            
            if (top_ptr->next_ptr==NULL)
            {
               cout << "Error in convexhull::Graham()" << endl;
               exit(-1);
            }

            // p1_ptr and p2_ptr point to top 2 points on stack
            
            tsPoint* p1_ptr=top_ptr->next_ptr->p_ptr;
            tsPoint* p2_ptr=top_ptr->p_ptr;

            if ( Left( p1_ptr->v , p2_ptr->v, P[i].v ) ) 
            {
               top_ptr = Push ( &P[i], top_ptr );
               i++;
            } 
            else    
            {
               top_ptr = Pop( top_ptr );
            }

//            cout << "Stack at bottom of while loop, i = " << i
//                 << " vnum = " << P[i].vnum << endl;
//            PrintStack(top_ptr);
//            outputfunc::newline();
         }
         return top_ptr;
      }

   void	Copy(int i,int j)
      {
         P[j].v[X] = P[i].v[X];
         P[j].v[Y] = P[i].v[Y];
         P[j].vnum = P[i].vnum;
         P[j].delete_flag = P[i].delete_flag;
      }

/*---------------------------------------------------------------------
  Returns twice the signed area of the triangle determined by a,b,c.
  The area is positive if a,b,c are oriented ccw, negative if cw,
  and zero if the points are collinear.
  ---------------------------------------------------------------------*/

   double Area2(tPointd a,tPointd b,tPointd c)
      {
         return (b[X] - a[X]) * (c[Y] - a[Y]) -
            (c[X] - a[X]) * (b[Y] - a[Y]);
      }
   
   int AreaSign(tPointd a,tPointd b,tPointd c)
      {
         double area2 = ( b[0] - a[0] ) * (double)( c[1] - a[1] ) -
            ( c[0] - a[0] ) * (double)( b[1] - a[1] );

         /* The area should be an integer. */
         if      ( area2 >  0.5 ) return  1;
         else if ( area2 < -0.5 ) return -1;
         else                     return  0;
      }

/*---------------------------------------------------------------------*/

   void PrintPoints( void )
      {
         cout << "Point printout:" << endl;
         for(int i = 0; i < npoints; i++ )
         {
            cout << "i = " << i << " point = " << P[i] << endl;
         }
      }

   void PrintPostscript(tsStack* t_ptr)
      {
         double xmin, ymin, xmax, ymax;

         xmin = xmax = P[0].v[X];
         ymin = ymax = P[0].v[Y];
         for (int i = 1; i < npoints ; i++) {
            if      ( P[i].v[X] > xmax ) xmax = P[i].v[X];
            else if ( P[i].v[X] < xmin ) xmin = P[i].v[X];
            if      ( P[i].v[Y] > ymax ) ymax = P[i].v[Y];
            else if ( P[i].v[Y] < ymin ) ymin = P[i].v[Y];
         }

         /* PostScript header */
         printf("%%!PS\n");
         printf("%%%%Creator: graham.c (Joseph O'Rourke)\n");
         printf("%%%%BoundingBox: %d %d %d %d\n", int(xmin), int(ymin), 
                int(xmax), int(ymax));
         printf("%%%%EndComments\n");
         printf(".00 .00 setlinewidth\n");
         printf("%d %d translate\n", int(-xmin+72), int(-ymin+72) );
         /* The +72 shifts the figure one inch from the lower left corner */

         /* Draw the points as little circles. */
         printf("newpath\n");
         printf("\n%%Points:\n");
         for (int i = 0; i < npoints; i++)
            printf("%d\t%d\t1  0  360\tarc\tstroke\n", 
                   basic_math::round(P[i].v[X]), 
                   basic_math::round(P[i].v[Y]));
         printf("closepath\n");

         /* Draw the polygon. */
         printf("\n%%Hull:\n");
         printf("newpath\n");
         printf("%d\t%d\tmoveto\n", 
                basic_math::round(t_ptr->p_ptr->v[X]), 
                basic_math::round(t_ptr->p_ptr->v[Y]));
         while (t_ptr != NULL) 
         {
            printf("%d\t%d\tlineto\n", 
                   basic_math::round(t_ptr->p_ptr->v[X]), 
                   basic_math::round(t_ptr->p_ptr->v[Y]));
            t_ptr = t_ptr->next_ptr;
         }
         printf("closepath stroke\n");
         printf("showpage\n%%%%EOF\n");
      }

// ---------------------------------------------------------------------
// Overload << operator:

   ostream& operator<< (ostream& outstream,const tsPoint& p)
      {
         outstream << endl;
         outstream << "&p = " << &p << endl;
         outstream << "vnum = " << p.vnum << endl;
         outstream << "v = ( " << p.v[0] << "," 
                   << p.v[1] << " )" << endl;
         outstream << "delete_flag = " << p.delete_flag << endl;
         return outstream;
      }

// ---------------------------------------------------------------------
// Method convex_hull_poly calls Graham's convex hull algorithm on the
// npoints integer valued points contained within input STL vector
// pixel_vertex.  This method is intended to be reasonably high-level
// and a more user friendly interface than others in this convexhull
// namespace.  It returns a dynamically generated polygon with integer
// valued vertices corresponding to the convex hull of all the input
// points.  It also reorders the entries within pixel_vertex so that
// its first nhull_points members lie on the hull.

   polygon* convex_hull_poly(vector<threevector>& pixel_vertex)
      {
         vector<int> vertex_order(pixel_vertex.size());
         return convex_hull_poly(pixel_vertex,vertex_order);
      }

// This overloaded version of convex_hull_poly reorders the entries
// within both the pixel_vertex and its corresponding vertex_order STL
// vectors so that their first null_points members lie on the hull:

   polygon* convex_hull_poly(
      vector<threevector>& pixel_vertex,vector<int>& vertex_order)
      {
         Initialize_Points(pixel_vertex);
         FindLowest();
         Sort_Points();	// Sort points according to angle
         Squash();
         tsStack* top_ptr=Graham();

// Dynamically generate convex hull polygon from pixels lying along
// the hull:
   
         polygon* hull_poly_ptr=NULL;
         if (top_ptr != NULL)
         {
            unsigned int nhull_points=Stacklength(top_ptr);
            vector<threevector> hull_vertex;

            while (top_ptr != NULL) 
            { 
               hull_vertex.push_back(threevector(
                  top_ptr->p_ptr->v[0],top_ptr->p_ptr->v[1]));
//               cout << "npnt = " << npnt
//                    << " hull_vertex = " << hull_vertex[npnt] << endl;
               top_ptr=top_ptr->next_ptr;
            }
            hull_poly_ptr=new polygon(hull_vertex);

// Reorder points within pixel_vertex STL vector so that its first
// nhull_points members correspond to actual hull points.  Make
// corresponding rearrangements on entries within vertex_order STL
// vector:

            vector<int> label;
            for (unsigned int n=0; n<pixel_vertex.size(); n++)
            {
               label.push_back(PMAX);
            }
            
            for (unsigned int m=0; m<nhull_points; m++)
            {
               for (unsigned int n=0; n<pixel_vertex.size(); n++)
               {
                  if (pixel_vertex[n]==hull_vertex[m])
                  {
                     label[n]=m;
                     break;
                  }
               } // loop over n index
            } // loop over m index
            
            templatefunc::Quicksort(label,pixel_vertex,vertex_order);
         } // top_ptr != NULL conditional
         delete top_ptr;
         return hull_poly_ptr;   
      }

// ---------------------------------------------------------------------
// This overloaded version of the convex_hull_poly method takes in a
// linked list containing pixel coordinates for a set of vertices.  It
// returns an integer pair valued polygon containing the hull of these
// vertices.

   polygon* convex_hull_poly(const linkedlist* pixel_list_ptr)
      {
         if (pixel_list_ptr != NULL)
         {
            int npoints=pixel_list_ptr->size();

            if (npoints==0)
            {
               return NULL;
            }
            else
            {
               int n=0;
               vector<threevector> pixel_vertex(npoints);
               const mynode* curr_pixel_ptr=pixel_list_ptr->get_start_ptr();
               while (curr_pixel_ptr != NULL)
               {
                  int px=basic_math::round(curr_pixel_ptr->get_data().
                                           get_var(0));
                  int py=basic_math::round(curr_pixel_ptr->get_data().
                                           get_var(1));
                  pixel_vertex[n++]=threevector(px,py,0);
                  curr_pixel_ptr=curr_pixel_ptr->get_nextptr();
               }
               return convex_hull_poly(pixel_vertex);
            }
         }
         else
         {
            return NULL;
         }
      }

   polygon* convex_hull_poly(const Linkedlist<pair<int,int> >* 
                             pixel_list_ptr)
      {
         if (pixel_list_ptr != NULL)
         {
            int npoints=pixel_list_ptr->size();

            if (npoints==0)
            {
               return NULL;
            }
            else
            {
               int n=0;
               vector<threevector> pixel_vertex(npoints);
               const Mynode<pair<int,int> >* curr_pixel_ptr=
                  pixel_list_ptr->get_start_ptr();
               while (curr_pixel_ptr != NULL)
               {
                  int px=curr_pixel_ptr->get_data().first;
                  int py=curr_pixel_ptr->get_data().second;
                  pixel_vertex[n++]=threevector(px,py,0);
                  curr_pixel_ptr=curr_pixel_ptr->get_nextptr();
               }
               return convex_hull_poly(pixel_vertex);
            }
         }
         else
         {
            return NULL;
         }
      }

} // convexhull namespace
