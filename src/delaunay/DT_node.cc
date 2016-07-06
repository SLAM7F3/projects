// ==========================================================================
// DT_NODE class member function definitions
// ==========================================================================
// Last modified on 12/14/05; 7/12/06
// ==========================================================================

#include "delaunay/Delaunay_point.h"
#include "delaunay/DT_list.h"
#include "delaunay/DT_node.h"
#include "math/threevector.h"

// ---------------------------------------------------------------------
// Constructors

DT_node::DT_node()
{
   vertices[0] = new Delaunay_point( 1  ,  0        );
   vertices[1] = new Delaunay_point(-0.5,  0.8660254);
   vertices[2] = new Delaunay_point(-0.5, -0.8660254);
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

DT_node::DT_node(DT_node* f, Delaunay_point *c, Index i)
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

// ---------------------------------------------------------------------
DT_node::Index DT_node::conflict(Delaunay_point *p)
{
   switch(flag.is_infinite())
   {
      case 4:
         return 0;
      case 3:
         return 1;
      case 2:
         return ( ( *p - *vertices[0]  ) * 
                  ( *vertices[1] + *vertices[2] ) >= 0 );
      case 1:
         return ( (flag.is_last_finite())
                  ?(	(( *p - *vertices[2] ) ^ 
                         ( *vertices[2] - *vertices[0] )) >=0)
                  :(	(( *p - *vertices[0] ) ^ 
                         ( *vertices[0] - *vertices[1] )) >=0));
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

// ---------------------------------------------------------------------
DT_node* DT_node::find_conflict(Delaunay_point* p)
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

// ---------------------------------------------------------------------
void DT_node::output(Network<threevector*>* vertex_network_ptr)
{
   DT_list* l;
   if ( flag.is_dead() ) 
   {
      for ( l=sons; l; l = l->next )
      {
         if ( l->key->nb != nb ) 
         {
            l->key->nb = nb;
            l->key->output(vertex_network_ptr);
         }
      }
      return;
   }

   if (neighbors[0]->nb != nb)
   {
      if ( ! flag.is_infinite() )  
      {
         vertices[1]->lineto(vertices[2],vertex_network_ptr);
      }
   }
   
   if (neighbors[1]->nb != nb)
   {
      if ( ! flag.is_infinite() )  
      {
         vertices[2]->lineto(vertices[0],vertex_network_ptr);
      }
      else if ( ( flag.is_infinite() == 1) && ( flag.is_last_finite()))
      {
         vertices[2]->lineto(vertices[0],vertex_network_ptr);
      }
   }
   
   if (neighbors[2]->nb != nb)
   {
      if ( ! flag.is_infinite() )
      {
         vertices[0]->lineto(vertices[1],vertex_network_ptr);
      }
      else if ( ( flag.is_infinite() == 1) && (!flag.is_last_finite()))
      {
         vertices[0]->lineto(vertices[1],vertex_network_ptr);
      }
   }
}
