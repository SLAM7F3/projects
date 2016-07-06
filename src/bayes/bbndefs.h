// bbndefs.h: prototypes and misc defs for Bayesian belief network code
//   T. Wallace, 5/1/00

extern "C"
{
   
#ifndef BBNDEFS_H
#define BBNDEFS_H

#ifndef DEBUG
#define DEBUG
#endif

#include "bbn.h"

NETNODE *create_node(int size, int psize, const char *name);
void free_node(NETNODE *n);
int init_netvect(NETVECT *nv, int size);
void free_netvect(NETVECT *nv);
int init_netmatrix(NETMATRIX *nm, int nrows, int ncols);
void free_netmatrix(NETMATRIX *nm);
int add_statename(NETNODE *n, int statenum, const char *name);
void build_transpose(NETNODE *n);
int nvcopy(NETVECT *dest, NETVECT *src);
#ifdef DEBUG
void dump_network(NETNODE *root);
void dump_node(NETNODE *clouds);
void dump_linkname(const char *namestr,NETNODE *n, const char *nullstr);
void dump_vect(const char *namestr,NETVECT *v);
#endif

int revise_belief(NETNODE *node);
int matmult(NETMATRIX *m,NETVECT *src,NETVECT *dest);
int term_product(NETVECT *s1, NETVECT *s2, NETVECT *dest);
int term_quotient(NETVECT *s1, NETVECT *s2, NETVECT *dest);
int normalize(NETVECT *v);
int update_node(NETNODE *node, NETNODE *sender);
int submit_evidence(NETNODE *node, NETVECT *evidence);

#define NNULL (NETNODE *)NULL

#endif // bbndefs.h
   
}
