/***************************************************************************+
+                                                                           +
+  Delaunay-tree                                                            +
+                                                                           +
+  Copyright (c) 1993  by  INRIA Prisme Project                             +
+  2004 route des Lucioles BP 93 06902 Sophia Antipolis Cedex               +
+  All rights reserved.                                                     +
+                                                                           +
****************************************************************************/

/**************************************************************************+

+ GEOMETRIC OBJECT :                                                       +
+ The Delaunay tree is a randomized geometric data structure computing the  +
+ Delaunay triangulation                                                   +
+ This structure holds insertion and queries. Deletions are not supported  +
+ in this version                                                          +
+                                                                          +

+ INTERNAL IMPLEMENTATION :                                                +
+ See :                                                                    +
+ J.-D. Boissonnat and M. Teillaud. On the randomized construction of the  +
+ Delaunay tree. Theoret. Comput. Sci. 112:339--354, 1993.                 +
+ O. Devillers, S. Meiser and M. Teillaud. Fully dynamic Delaunay          +
+ triangulation in logarithmic expected time per operation. Comput. Geom.  +
+ Theory Appl. 2(2):55-80, 1992.                                           +
+ O. Devillers. Robust and efficient implementation of the Delaunay tree.  +
+ INRIA Research Report 1619, 1992.                                        +
+**************************************************************************/

#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "math/prob_distribution.h"
#include "general/stringfuncs.h"
#include "math/threevector.h"
#include "math/twovector.h"

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

class Delaunay_tree;
class DT_node;
class DT_list;
class point;
typedef unsigned char Index;	// used for flag and Index in array

int counter=0;
ofstream outstream;
vector<double> edge_length;

class Delaunay_tree
{
   public :

      Delaunay_tree();                     // initialization as empty
      ~Delaunay_tree();                       // destructor
      Delaunay_tree& operator+=(point*);      // insertion
      void output();			   // the Delaunay triangulation


   private :
      // number of the current operation
      int nb;
   // the root of the delaunay_tree
      DT_node* root;
};

class point
{
   private :
      double x;
      double y;
   public :
      point(double xx, double yy) { x=xx; y=yy; }
      double X() {return x;}
      double Y() {return y;}
      void lineto(point*);
      pair<threevector,threevector> triangle_edge_vertices(point* p);
      friend  point  operator+(point, point);
      friend  point  operator-(point, point);
      friend  double  operator*(point, point);
      friend  double  operator^(point, point);
};

pair<threevector,threevector> point::triangle_edge_vertices(point* p_ptr)
{ 
   threevector V1(this->x,this->y);
   threevector V2(p_ptr->x,p_ptr->y);
   return pair<threevector,threevector>(V1,V2);
}

void point::lineto(point* p_ptr)
{ 
   const double SMALL_NEG=-0.001;
   threevector V1(this->x,SMALL_NEG,this->y);
   threevector V2(p_ptr->x,SMALL_NEG,p_ptr->y);
   edge_length.push_back( (V2-V1).magnitude() );
   outstream << "0   " << counter << "  0  "
             << V1.get(0) << "   " << V1.get(1) << "   " << V1.get(2) << "   "
             << V2.get(0) << "   " << V2.get(1) << "   " << V2.get(2) << "   "
             << "1" << endl;
   counter++;
}

/*
void point::lineto(point* p_ptr)
{ 
   const double SMALL_NEG=-0.001;
   threevector V1(this->x,SMALL_NEG,this->y);
   threevector V2(p_ptr->x,SMALL_NEG,p_ptr->y);
   edge_length.push_back( (V2-V1).magnitude() );
   outstream << "0   " << counter << "  0  "
             << V1.get(0) << "   " << V1.get(1) << "   " << V1.get(2) << "   "
             << V2.get(0) << "   " << V2.get(1) << "   " << V2.get(2) << "   "
             << "1" << endl;
   counter++;
}
*/

class DT_flag
{
   public :
      friend class DT_node;
      friend class Delaunay_tree;

   private :
      Index f;
      DT_flag()             { f = (Index) 0 ; }
      void infinite(int i)     { f |= (Index) i ; }
      void last_finite()       { f |= 8;}
      void kill()              { f |= 16;}
      Index is_infinite()   	 { return f & 7;}
      Index is_last_finite()   { return f & 8;}
      Index is_dead()      	 { return f & 16;}
      Index is_degenerate()    { return f & 32;}
};

class DT_list
{
   public :
      friend class DT_node;
      friend class Delaunay_tree;

   private :
      DT_list*		next;
      DT_node*		key;
      DT_list(DT_list* l, DT_node* k) {next=l; key=k;}
      ~DT_list();
};

class DT_node
{
   public :
      friend class Delaunay_tree;

   private :
      // the first vertex is the creator, that is finite
      // except for the root and its neighbors
      DT_flag			flag;
   unsigned int	nb;
   point*			vertices[3];
   DT_node*        neighbors[3];
   DT_list*		sons;
   DT_node();					// initialize the root
   DT_node(DT_node*, Index);	// initialize nowhere
   DT_node(DT_node*, point*, Index);
   // father, creator, direction of stepfather
   Index conflict(point *); 
   // true if the point is inside the (closed) circumdisk
   DT_node* find_conflict(point* p);
   // return an alive node in conflict
   void output();
   Index cw_neighbor_Index(point *p)
      { return ((p==vertices[0]) ? 2 : ((p==vertices[1]) ? 0 : 1)); }
   Index neighbor_Index(DT_node *n)
      { return ( (neighbors[0]==n)?0:((neighbors[1]==n)?1:2) );}
};

inline point   operator+(point a, point b) { return point(a.x+b.x, a.y+b.y);}
inline point   operator-(point a, point b) { return point(a.x-b.x, a.y-b.y);}
inline double  operator*(point a, point b) { return a.x*b.x + a.y*b.y;}
inline double  operator^(point a, point b) { return a.x*b.y - a.y*b.x;}


DT_list::~DT_list()
{ 
   DT_list *p,*q;
   if (!next) return;
   for ( p = this, q = this->next; q ; ) 
   {
      p->next = NULL;
      delete p;
      p = q;
      q = q->next ;
   }
}

Index DT_node::conflict(point *p)
{
   switch(flag.is_infinite())
   {
      case 4:
         return 0;
      case 3:
         return 1;
      case 2:
         return ( ( *p - *vertices[0]  ) * ( *vertices[1] + *vertices[2] ) >= 0 );
      case 1:
         return ( (flag.is_last_finite())
                  ?(	(( *p - *vertices[2] ) ^ ( *vertices[2] - *vertices[0] )) >=0)
                  :(	(( *p - *vertices[0] ) ^ ( *vertices[0] - *vertices[1] )) >=0));
      case 0:
         // compute the det 4*4 column: x,y,x**2+y**2,1 for p and vertices [0,1,2]
         double x,y;
         double x0,y0;
         double x1,y1;
         double x2,y2;
         double z1,z2;
         double alpha,beta,gamma;
         x  = p->X();
         y  = p->Y();
         x0 = vertices[0]->X();
         y0 = vertices[0]->Y();
         x1 = vertices[1]->X();
         y1 = vertices[1]->Y();
         x2 = vertices[2]->X();
         y2 = vertices[2]->Y();
         x1-=x0;
         y1-=y0;
         x2-=x0;
         y2-=y0;
         x-=x0;
         y-=y0;
         z1=(x1*x1)+(y1*y1);
         z2=(x2*x2)+(y2*y2);
         alpha=(y1*z2)-(z1*y2);
         beta=(x2*z1)-(x1*z2);
         gamma=(x1*y2)-(y1*x2);
         return ((alpha*x)+(beta*y)+gamma*((x*x)+(y*y))  <= 0);
   }
}

DT_node::DT_node()
{
   vertices[0] = new point( 1  ,  0        );
   vertices[1] = new point(-0.5,  0.8660254);
   vertices[2] = new point(-0.5, -0.8660254);
   flag.infinite(3);;
   nb = 0;
   sons = NULL;
}

DT_node::DT_node(DT_node* root, Index i)
{
   vertices[0] = root->vertices[0] ;
   vertices[1] = root->vertices[1] ;
   vertices[2] = root->vertices[2] ;
   flag.infinite(4);
   nb = 0;
   sons = NULL;
   neighbors[i] = root ;
   root->neighbors[i] = this ;
}

DT_node::DT_node(DT_node* f, point *c, Index i)
{
// the triangle is created in ccw order
// circumdisk and flatness are not computed
   switch ( f->flag.is_infinite() )
   {
      case 0:  flag.infinite(0);
         break;
      case 1: if (f->flag.is_last_finite() ) flag.infinite( (i==1) ? 0 : 1 );
      else                        flag.infinite( (i==2) ? 0 : 1 );
      if ( flag.is_infinite() )
         if ( f->flag.is_last_finite()  )
         { if ( i==0 ) flag.last_finite(); }
         else
         { if ( i==1 ) flag.last_finite(); }
      break;
      case 2: flag.infinite( (i==0) ? 2 : 1 );
         if (i==1) flag.last_finite();
         break;
      case 3:  flag.infinite(2);
         break;
   }
   nb = 0;
   sons = NULL;
   f->sons = new DT_list(f->sons, this);
   f->neighbors[i]->sons = new DT_list(f->neighbors[i]->sons, this);
   f->neighbors[i]->neighbors[ f->neighbors[i]->neighbor_Index(f) ] = this ;
   vertices[0] = c;
   neighbors[0] = f->neighbors[i];
   switch (i) {
      case 0:
         vertices[1] = f->vertices[1];
         vertices[2] = f->vertices[2];
         break;
      case 1:
         vertices[1] = f->vertices[2];
         vertices[2] = f->vertices[0];
         break;
      case 2:
         vertices[1] = f->vertices[0];
         vertices[2] = f->vertices[1];
         break;
   }
}

DT_node* DT_node::find_conflict(point* p)
{
   DT_list* l;
   DT_node* n;
   if( ! conflict(p) ) return NULL;
   if ( ! flag.is_dead() ) return this;
   for ( l=sons; l; l = l->next )
      if ( l->key->nb != nb ) {
         l->key->nb = nb;
         n=l->key->find_conflict(p);
         if (n) return n;
      }
   return NULL;
}

void Delaunay_tree::output()
// output the Delaunay triangulation
{
   root->nb = ++nb;
   root->output();
}


void DT_node::output()
{
   DT_list* l;
   if ( flag.is_dead() ) 
   {
      for ( l=sons; l; l = l->next )
      {
         if ( l->key->nb != nb ) 
         {
            l->key->nb = nb;
            l->key->output();
         }
      }
      return;
   }
   if (neighbors[0]->nb != nb)
      if ( ! flag.is_infinite() )  vertices[1]->lineto(vertices[2]);
   if (neighbors[1]->nb != nb)
      if ( ! flag.is_infinite() )  vertices[2]->lineto(vertices[0]);
      else if ( ( flag.is_infinite() == 1)
                &&( flag.is_last_finite()))vertices[2]->lineto(vertices[0]);
   if (neighbors[2]->nb != nb)
      if ( ! flag.is_infinite() )vertices[0]->lineto(vertices[1]);
      else if ( ( flag.is_infinite() == 1)
                &&(!flag.is_last_finite()))vertices[0]->lineto(vertices[1]);
}

Delaunay_tree::Delaunay_tree()         // initialization as empty
{
   nb = 0;
   root = new DT_node();
   new DT_node(root, 0);
   new DT_node(root, 1);
   new DT_node(root, 2);
   root->neighbors[0]->neighbors[1] =  root->neighbors[1];
   root->neighbors[0]->neighbors[2] =  root->neighbors[2];
   root->neighbors[1]->neighbors[0] =  root->neighbors[0];
   root->neighbors[1]->neighbors[2] =  root->neighbors[2];
   root->neighbors[2]->neighbors[0] =  root->neighbors[0];
   root->neighbors[2]->neighbors[1] =  root->neighbors[1];
}

Delaunay_tree::~Delaunay_tree()                       // destructor
{
   nb++;
}


Delaunay_tree& Delaunay_tree::operator+=(point* p)      // insertion
{
   DT_node* n;
   DT_node* created;
   DT_node* last;
   DT_node* first;
   point* q;
   point* r;
   Index i;

   root->nb = ++nb;
   if ( ! ( n = root->find_conflict(p) ) ) return *this ;
   // test if p is already inserted
   for ( i=0; (int) i < 3- (int) n->flag.is_infinite(); i++ )
      if ( ( p->X()==n->vertices[i]->X() )&&( p->Y()==n->vertices[i]->Y() ) )
         return *this;
   n->flag.kill();
   // we will turn cw around first vertex of n, till next triangle
   // is not in conflict
   q = n->vertices[0];
   while( n->neighbors[ i=n->cw_neighbor_Index(q) ]->conflict(p) )
   {n = n->neighbors[i];n->flag.kill();}

   first = last = new DT_node(n,p,i);
   // we will turn cw around r, till next triangle is not in conflict
   r = n->vertices[(((int) i)+2)%3];
   while( 1){
      i = n->cw_neighbor_Index(r);
      if (n->neighbors[i]->flag.is_dead()){
         n = n->neighbors[i]; continue;
      }
      if (n->neighbors[i]->conflict(p) ){
         n = n->neighbors[i];
         n->flag.kill();
         continue;
      }
      break;
   }

   while ( 1 ) {
      // n is killed by p
      // n->neighbors[i] is not in conflict with p
      // r is vertex i+1 of n
      created = new DT_node(n,p,i);
      created->neighbors[2]=last;
      last->neighbors[1]=created;
      last=created;
      r = n->vertices[(((int) i)+2)%3];   // r turn in n ccw
      if (r==q) break;
      // we will turn cw around r, till next triangle is not in conflict
      while( 1){
         i = n->cw_neighbor_Index(r);
         if (n->neighbors[i]->flag.is_dead()){
            n = n->neighbors[i]; continue;
         }
         if (n->neighbors[i]->conflict(p) ){
            n = n->neighbors[i];
            n->flag.kill();
            continue;
         }
         break;
      }
   }
   first->neighbors[2]=last;
   last->neighbors[1]=first;
   return *this;
}

vector<threevector> read_feature_info_from_file(string input_filename)
{
   if (!filefunc::ReadInfile(input_filename)) exit(-1);

   const int n_fields=5;
   double X[n_fields];
   vector<double> curr_time;
   vector<int> feature_ID,pass_number;
   vector<threevector> UVW;
   curr_time.reserve(filefunc::text_line.size());
   feature_ID.reserve(filefunc::text_line.size());
   pass_number.reserve(filefunc::text_line.size());
   UVW.reserve(filefunc::text_line.size());

   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      stringfunc::string_to_n_numbers(n_fields,filefunc::text_line[i],X);
      curr_time.push_back(X[0]);
      feature_ID.push_back(basic_math::round(X[1]));
      pass_number.push_back(basic_math::round(X[2]));
      double U=X[3];
      double V=X[4];
      double W=0;

      if (nearly_equal(curr_time.back(),0))
         UVW.push_back(threevector(U,V,W));
   } // loop over index i labeling ascii file filefunc::text_line number

//   for (unsigned int i=0; i<curr_time.size(); i++)
//   {
//         cout << "time = " << curr_time[i]
//              << " ID = " << feature_ID[i]
//              << " pass = " << pass_number[i]
//              << " UVW = " << UVW[i] << endl;
//   }
   return UVW;
}

int main()
{
//   string input_filename="reduced_features.txt";
//   cout << "Enter name of file containing feature sites:" << endl;
//   cin >> input_filename;
   vector<threevector> UVW;
//   vector<threevector> UVW=read_feature_info_from_file(input_filename);

   UVW.push_back(threevector(0,0));
   UVW.push_back(threevector(1,0));
   UVW.push_back(threevector(0,1));
   UVW.push_back(threevector(2,0));
   UVW.push_back(threevector(2,2));

   cout << "UVW.size() = " << UVW.size() << endl;

   Delaunay_tree DT;

   for (int i=0; i<UVW.size(); i++)
   {
      cout << "i = " << i << " UVW = " << UVW[i] << endl;
      DT += new point(UVW[i].get(0),UVW[i].get(1));
   }
   
   string Delaunay_filename="Delaunay.txt";
   filefunc::openfile(Delaunay_filename,outstream);
   DT.output();
   filefunc::closefile(Delaunay_filename,outstream);

/*
   prob_distribution prob(edge_length,30);
   cout << "Median Delaunay triangle edge length = "
        << prob.median() << endl;
   cout << "Avg Delaunay triangle edge length = "
        << mathfunc::mean(edge_length) << " +/- " 
        << mathfunc::std_dev(edge_length) << endl;
*/

}

