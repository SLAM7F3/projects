// ==================================================================
// bbnupdate.cc: Routines for updating Bayesian belief networks
// T. Wallace, 5/1/00

extern "C" 
{
   
#include "bbndefs.h"

#define EXTERNAL extern
#include "debug_wallace.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// ------------------------------------------------------------------
/* create NETNODE and return pointer to it.  size is the number
   of states in this node, and psize the number in its parent.
   If this is the root node, psize should be anything <= 0.
   To prevent memory leaks, following convention is followed:
   if a NETNODE *n exists, then n->nodename, n->statename,
   n->belief, n->pi, n->external_lambda,
   and n->external_lambda exist and all of them must be freed,
   in reverse order.  If psize <= 0, this is the root node,
   and n->outgoing_lambda.dat is set to NULL as a flag.  Otherwise
   we free n->outgoing_lambda, n->incoming_pi,
   n->CP, and n->CPtrans
   There should be no memory leaks even if a malloc fails; we clean up
*/

NETNODE *create_node(int size, int psize, const char *name)
{  
   NETNODE *n;
   int i;

   TRACE(5,(DF,"called create_node(%d,%d,%s)\n",size,psize,name));
   if ((n = (NETNODE *)malloc(sizeof(NETNODE))) == NNULL) {
      return NNULL;
   }
#ifdef DEBUG
   if ((n->nodename = (char *)malloc(strlen(name)+1)) == NULL) {
      free(n);
      return NNULL;
   }
   strcpy(n->nodename,name);
   if ((n->statename = (char **)malloc(sizeof(char *) * size)) == NULL) {
      free(n->nodename);
      free(n);
      return NNULL;
   }
   
#endif

   n->len = size;
   if (!init_netvect(&(n->belief), size)) {
#ifdef DEBUG
      free(n->statename); free(n->nodename);
#endif
      free(n);
      return NNULL;
   }
   if (!init_netvect(&(n->pi), size)) {
      free_netvect(&(n->belief));
#ifdef DEBUG
      free(n->statename); free(n->nodename);
#endif
      free(n);
      return NNULL;
   }
   if (!init_netvect(&(n->external_lambda), size)) {
      free_netvect(&(n->pi)); free_netvect(&(n->belief));
#ifdef DEBUG
      free(n->statename); free(n->nodename);
#endif
      free(n);
      return NNULL;
   }
   if (!init_netvect(&(n->lambda), size)) {
      free_netvect(&(n->external_lambda));
      free_netvect(&(n->pi)); free_netvect(&(n->belief));
#ifdef DEBUG
      free(n->statename); free(n->nodename);
#endif
      free(n);
      return NNULL;
   }
   /* don't need matrices if this is the root node (no parent) */
   if (psize <= 0) { /* set flag for no-parent case */
      n->outgoing_lambda.dat = (REAL *)NULL;
   } else {
/*	The CP matrix relating this node to its parent is stored here.
	Hence anything coming/going from/to the parent must have the
	parent's dimension psize
*/
      if (!init_netvect(&(n->outgoing_lambda), psize)) {
         /* since n->outgoing_lambda is NULL, free_node will work */
         free_node(n);
         return NNULL;
      }
      for (i=0; i<psize; i++) {
         n->outgoing_lambda.dat[i] = 1.0;
      }
      if (!init_netvect(&(n->incoming_pi), psize)) {
         free_netvect(&(n->outgoing_lambda));
         free_node(n);
         return NNULL;
      }
      if (!init_netmatrix(&(n->CP), psize, size)) {
         free_netvect(&(n->incoming_pi));
         free_netvect(&(n->outgoing_lambda));
         free_node(n);
         return NNULL;
      }
      if (!init_netmatrix(&(n->CPtrans), size, psize)) {
         free_netmatrix(&(n->CP));
         free_netvect(&(n->incoming_pi));
         free_netvect(&(n->outgoing_lambda));
         free_node(n);
         return NNULL;
      }
   }
   n->parent = n->next_sibling = n->first_child = NNULL;
   for (i=0; i<size; i++) {
      n->external_lambda.dat[i] = 1.0;
   }
   return n;
}
   

// ------------------------------------------------------------------
/* To prevent memory leaks, following convention is followed:
   if a NETNODE *n exists, then n->nodename, n->statename,
   n->belief, n->pi, n->external_lambda,
   and n->external_lambda exist and all of them must be freed,
   in reverse order.  If n->outgoing_lambda.dat == NULL, this is the
   root node.  Otherwise we free n->outgoing_lambda,
   n->incoming_pi, n->CP, and n->CPtrans, in reverse order again
*/

void free_node(NETNODE *n)
{ 
   int size, i;
   TRACE(10,(DF,"free_node(%p): outgoing_lambda.dat is %p\n",
             n,n->outgoing_lambda.dat));
#ifdef DEBUG
//   if (debug >= 1000) {
//      fprintf(DF,
//              "Warning: children of next node may print out as garbage since already freed!\n");
//      dump_node(n);
//  }
#endif
   size = n->belief.nvals;
   if (n->outgoing_lambda.dat != (REAL *)NULL) {
      free_netmatrix(&(n->CPtrans));
      free_netmatrix(&(n->CP));
      free_netvect(&(n->incoming_pi));
      free_netvect(&(n->outgoing_lambda));
   }
   free_netvect(&(n->lambda)); free_netvect(&(n->external_lambda));
   free_netvect(&(n->pi)); free_netvect(&(n->belief));
#ifdef DEBUG
   for (i = 0; i < size; i++) {
      free(n->statename[i]);
   }
   free(n->statename); free(n->nodename);
#endif
   free(n);
}


// ------------------------------------------------------------------
/* note that init_netvect does not create the netvect variable itself,
   and so init_netvect/free_netvect is not quite the analog of
   create_node/free_node. */

int init_netvect(NETVECT *nv, int size)
{
   if (size <= 0) return 0;
   if ((nv->dat = (REAL *)malloc(sizeof(REAL) * size)) == NULL) {
      return 0;
   }
   nv->nvals = size;
   return 1;
}

// ------------------------------------------------------------------
/* note that init_netmatrix does not create the netmatrix variable it
   self, and so init_netmatrix/free_netmatrix is not quite the analog of
   create_node/free_node. */
// There should be no memory leaks even if a malloc fails; we clean up

int init_netmatrix(NETMATRIX *nm, int nrows, int ncols)
{ 
   int i, j;

   if (nrows <= 0) return 0;
   if (ncols <= 0) return 0;

   if ((nm->mat = (REAL **)malloc(sizeof(REAL *) * nrows)) == NULL) {
      return 0;
   }
   for (i=0; i<nrows; i++) {
      if ((nm->mat[i] = (REAL *)malloc(sizeof(REAL) * ncols)) == NULL) {
         for (j=i-1; j>=0; j--)
            free(nm->mat[j]);
         free(nm->mat);
         return 0;
      }
   }
   nm->nrows = nrows;
   nm->ncols = ncols;
   return 1;
}


// ------------------------------------------------------------------
/* note that free_netvect does not free the netvect variable itself,
   and so init_netvect/free_netvect is not quite the analog of
   create_node/free_node. */

void free_netvect(NETVECT *nv)
{
   TRACE(10,(DF,"free_netvect(%p)\n",nv));
   free(nv->dat);
   nv->dat = (REAL *)NULL;
}

// ------------------------------------------------------------------
/* note that free_netmatrix does not free the netmatrix variable itself,
   and so init_netmatrix/free_netmatrix is not quite the analog of
   create_node/free_node. */
/* free stuff in reverse order, just in case it helps heap collection */

void free_netmatrix(NETMATRIX *nm)
{
   int i;

   TRACE(10,(DF,"free_netmatrix(%p)\n",nm));
   for (i=nm->nrows - 1; i >= 0; i--) {
      free(nm->mat[i]);
   }
   free(nm->mat);
}

// ------------------------------------------------------------------
/* save name of state ``statenum'' in node *n */

int add_statename(NETNODE *n, int statenum, const char *name)
{ 
#ifdef DEBUG
   if ((n->statename[statenum] = (char *)malloc(strlen(name)+1)) == NULL) {
      return 0;
   }
   strcpy(n->statename[statenum],name);
#endif
   return 1;
}


// ------------------------------------------------------------------
void build_transpose(NETNODE *n)
{
   int i, j;

   for (i=0; i<n->CP.nrows; i++) {
      for (j=0; j<n->len; j++) {
         n->CPtrans.mat[j][i] = n->CP.mat[i][j];
      }
   }
}

// ------------------------------------------------------------------
/* copy netvect src to dest */

int nvcopy(NETVECT *dest, NETVECT *src)
{	
   int i;

   if (src->nvals != dest->nvals) {
      return 0;
   } else {
      TRACE(100,(DF,"nvcopy: \n"));
      for (i=0; i<src->nvals; i++) {
         TRACE(100,(DF,"%.2f ",src->dat[i]));
         dest->dat[i] = src->dat[i];
      }
      TRACE(100,(DF,"\n"));
   }
   return 1;
}

#ifdef DEBUG

// ------------------------------------------------------------------
/* dump entire bayesian belief network with dump_node function */

void dump_network(NETNODE *root)
{ 
   TRACE(10,(DF,"dumping node %s\n",root->nodename));
   dump_node(root);
   if (root->first_child != NNULL) {
      TRACE(10,(DF,"dumping first_child %s\n",root->first_child->nodename));
      dump_network(root->first_child);
   }
   if (root->next_sibling != NNULL) {
      TRACE(10,(DF,"dumping next_sibling %s\n",root->next_sibling->nodename));
      dump_network(root->next_sibling);
   }
}


// ------------------------------------------------------------------
/* dump contents of this node */

void dump_node(NETNODE *n)
{		/* dump contents of this node */
   int i, j;
   const char *prefix;

   fprintf(stderr,"   BBN Node %s",n->nodename);
   if ( (n->len > 0) && strlen(n->statename[0]) ) {
      fprintf(stderr," (%s",n->statename[0]);
      for (i=1; i<n->len; i++) {
         fprintf(stderr,", %s",n->statename[i]);
      }
      fprintf(stderr,")");
   }
   if (n->parent != NNULL) {
      fprintf(stderr,"  [Parent: %s]",n->parent->nodename);
   } else {
      fprintf(stderr,"  [Root Node]");
   }
   dump_linkname("\n  [first_child: ",n->first_child,"NONE]");
   dump_linkname("  [next_sibling: ",n->next_sibling,"NONE]");
   dump_vect("\nPi =",&(n->pi));
   if (n->len > 4) {
      dump_vect("\nLambda =",&(n->lambda));
   } else {
      dump_vect("  Lambda =",&(n->lambda));
   }
   if (n->len > 4) {
      dump_vect("\nBel =",&(n->belief));
   } else {
      dump_vect("  Bel =",&(n->belief));
   }
   if (n->outgoing_lambda.dat != (REAL *)NULL) {
      /* have CP matrix (not root node) */
      dump_vect("\nIncoming_pi =",&(n->incoming_pi));
      if (n->len > 4) {
         dump_vect("\nOutgoing_lambda =",&(n->outgoing_lambda));
      } else {
         dump_vect("  Outgoing_lambda =",&(n->outgoing_lambda));
      }
   }
   dump_vect("\nExternal_lambda =",&(n->external_lambda));
   fprintf(stderr,"\n");
   if (n->outgoing_lambda.dat != (REAL *)NULL) {
      prefix = "CP = ";
      for (i=0; i<n->CP.nrows; i++) {
         fprintf(stderr,"%s(%.3f",prefix,n->CP.mat[i][0]);
         for (j=1; j<n->len; j++) {
            fprintf(stderr,",%.3f",n->CP.mat[i][j]);
         }
         fprintf(stderr,")\n");
         prefix = "     ";
      }
   }
   fprintf(stderr,"\n");
}


// ------------------------------------------------------------------
void dump_linkname(const char *namestr,NETNODE *n, const char *nullstr)
{
   fprintf(stderr,"%s",namestr);
   if (n == NNULL) {
      fprintf(stderr,"%s",nullstr);
   } else {
      fprintf(stderr,"%s]",n->nodename);
   }
}

// ------------------------------------------------------------------
void dump_vect(const char *namestr,NETVECT *v)
{
   int i;

   if (v->nvals > 0) {
//      if (debug > 1000) {
//         fprintf(stderr,"%s [%p]([%p]%.3f",namestr,v,v->dat,v->dat[0]);
//         for (i=1; i<v->nvals; i++) {
//            fprintf(stderr,",[%p]%.3f",v->dat+i,v->dat[i]);
//         }
         fprintf(stderr,")");
      } else {
         fprintf(stderr,"%s (%.3f",namestr,v->dat[0]);
         for (i=1; i<v->nvals; i++) {
            fprintf(stderr,",%.3f",v->dat[i]);
         }
         fprintf(stderr,")");
      }
   }
}

#endif

// ------------------------------------------------------------------
/* update belief in *node based on current *node data */

int revise_belief(NETNODE *node)
{ 
   NETNODE *child;
   int retval;

   TRACE(5,(DF,"Revise_belief: processing node %s\n",node->nodename));

   if (node->parent != NNULL) {
      matmult(&(node->CPtrans), &(node->incoming_pi), &(node->pi));
   }
   nvcopy(&(node->lambda),&(node->external_lambda));

   child = node->first_child;
   while (child != NNULL) {
#ifdef DEBUG
//      if (debug >= 10) {
//         fprintf(DF,"Revising lambda of %s considering child %s\n",
//                 node->nodename,child->nodename);
//         dump_vect("lambda",&(node->lambda));
//      }
#endif
      term_product(&(child->outgoing_lambda),&(node->lambda),&(node->lambda));
#ifdef DEBUG
//      if (debug >= 10) {
//         dump_vect(" --> ",&(node->lambda));
//         fprintf(DF,"\n");
//      }
#endif
      child = child->next_sibling;
   }
/* mathematically don't need to normalize lambda, but can have numerical
   problems where it just shrinks down to nothing if we don't! */
   if (!normalize(&(node->lambda))) return 0;
   term_product(&(node->lambda), &(node->pi), &(node->belief));
   retval = normalize(&(node->belief));
#ifdef DEBUG
//   if (debug > 1) dump_node(node);
#endif
   return retval;
}

// ------------------------------------------------------------------
/* matrix multiply such that ``dest = m * src'' src and dest must be
   distinct */

int matmult(NETMATRIX *m,NETVECT *src,NETVECT *dest)
{ 
   int i, j;
   REAL *s1, *s2, *dst;
   int nrows, ncols;

   ncols = m->ncols;
   nrows = m->nrows;
   if (src->nvals != ncols) {
      fprintf(stderr,"Bad col/src dimensions in matmult\n");
      return 0;
   }
   if (dest->nvals != nrows) {
      fprintf(stderr,"Bad row/dest dimensions in matmult\n");
      return 0;
   }
   dst = dest->dat;
   for (i=0; i<nrows; i++) {
      s1 = m->mat[i];
      s2 = src->dat;
      *dst = *s1++ * *s2++;
      for (j=1; j<ncols; j++) {
         *dst += *s1++ * *s2++;
      }
      dst++;
   }
   return 1;
}

// ------------------------------------------------------------------
/* term-by-term product of two vectors ``dest = s1 * s2'' */

int term_product(NETVECT *s1, NETVECT *s2, NETVECT *dest)
{  
   int i;

   if ( (s1->nvals != s2->nvals) || (s1->nvals != dest->nvals) ) {
      fprintf(stderr,"Bad dimensions in term_product\n");
      return 0;
   }
   for (i=0; i<s1->nvals; i++) {
      dest->dat[i] = s1->dat[i] * s2->dat[i];
   }
   return 1;
}

// ------------------------------------------------------------------
/* term-by-term quotient of two vectors ``dest = s1 / s2'' */

int term_quotient(NETVECT *s1, NETVECT *s2, NETVECT *dest)
{  
   int i;

   if ( (s1->nvals != s2->nvals) || (s1->nvals != dest->nvals) ) {
      fprintf(stderr,"Bad dimensions in term_quotient\n");
      return 0;
   }
   for (i=0; i<s1->nvals; i++) {
      if (s2->dat[i] > 1e-20) {
         dest->dat[i] = s1->dat[i] / s2->dat[i];
      } else {
         fprintf(stderr,"Val too small to divide by in term_quotient\n");
         return 0;
      }
   }
   return 1;
}

// ------------------------------------------------------------------
/* normalize v, assumed to be a discrete pdf, to sum to 1 */

int normalize(NETVECT *v)
{ 
   int i;
   double total = 0.0;

   for (i=0; i<v->nvals; i++) {
      total += v->dat[i];
   }
   if (total <= 0.0) {
      fprintf(stderr,"pdf sums to <= 0!\n");
      return 0;
   }
   TRACE(100,(DF,"normalize: "));
   for (i=0; i<v->nvals; i++) {
      v->dat[i] /= total;
      TRACE(100,(DF,"%.3f  ",v->dat[i]));
   }
   TRACE(100,(DF,"\n"));
   return 1;
}

// ------------------------------------------------------------------
/* recursive update of entire network when sender sends to node */

int update_node(NETNODE *node, NETNODE *sender)
{ 
   NETNODE *child;

   TRACE(10,(DF," *** Entering update_node at %s\n",node->nodename));

   if (!revise_belief(node)) {
      return 0;
   }

   if ( (node->parent != sender) && (node->parent != NNULL) ) {
      TRACE(10,(DF,"Sending lambda from %s to parent %s\n",
                node->nodename,node->parent->nodename));
      /* send lambda message to parent */
      matmult(&(node->CP), &(node->lambda), &(node->outgoing_lambda));
#ifdef DEBUG
//      if (debug >= 10) {
//         dump_vect("outgoing_lambda =",&(node->outgoing_lambda));
//         fprintf(DF,"\n");
//      }
#endif
      /* now ready to update parent */
      update_node(node->parent,node);
   }

   child = node->first_child;
   while (child != NNULL) {
      if (child != sender) {
         TRACE(10,(DF,"Propagating pi in %s\n",child->nodename));
         if (term_quotient(&(node->belief), &(child->outgoing_lambda),
                           &(child->incoming_pi))) {
#ifdef DEBUG
//            if (debug >= 10) {
//               dump_vect("incoming_pi",&(child->incoming_pi));
//               dump_vect(" = parent bel",&(node->belief));
//               dump_vect(" / outgoing_lambda",&(child->outgoing_lambda));
//               fprintf(DF,"\n");
//            }
#endif
            update_node(child,node);
         } else {
            fprintf(stderr,"term_quotient failure!\n");
            return 0;
         }
      }
      child = child->next_sibling;
   }
   return 1;
}

// ------------------------------------------------------------------
int submit_evidence(NETNODE *node, NETVECT *evidence)
{
#ifdef DEBUG
//   if (debug >= 10) {
//      fprintf(DF,"Submitting external evidence to %s\n",node->nodename);
//      dump_vect("external_lambda",&(node->external_lambda));
//   }
#endif
   term_product(evidence, &(node->external_lambda), &(node->external_lambda));
#ifdef DEBUG
//   if (debug >= 10) {
//      dump_vect(", becomes",&(node->external_lambda));
//      fprintf(DF,"\n");
//   }
#endif
   return update_node(node,NNULL);
}



