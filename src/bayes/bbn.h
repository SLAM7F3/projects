// bbn.h: data structure definitions for Bayesian belief networks
//   T. Wallace, 5/1/00

extern "C"
{
   
#ifndef BBN_H
#define BBN_H

#define REAL double

typedef struct netvect { /* belief vects */
  int nvals;
  REAL *dat;
} NETVECT;

typedef struct netmatrix { /* conditional prob matrices */
  int nrows, ncols;
  REAL **mat;
} NETMATRIX;

typedef struct netnode { /* belief network node */
#ifdef DEBUG
  char *nodename; /* string should identify this node */
  char **statename; /* list of ptrs to ``len'' strings identifying states */
#endif
  int len; /* length of belief vector */
  NETVECT belief, pi, incoming_pi;
  NETVECT external_lambda, lambda, outgoing_lambda;
  struct netnode *parent, *next_sibling, *first_child;
	/* the conditional probability matrix element
	 CP[i][j] is P{ child[j] | parent[i] } */
  NETMATRIX CP, CPtrans; /* store transpose also for speed */
} NETNODE;

#endif // bbn.h

}
