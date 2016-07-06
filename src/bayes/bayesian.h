// =================================================================
// Header for Bayesian belief network subroutines written in C
// =================================================================
// Last updated on 2/26/03
// =================================================================

#ifndef BAYESIAN_H
#define BAYESIAN_H

#include <malloc.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <math.h>
#include "bbndefs.h"

// #define EXTERNAL
#include "debug_wallace.h"

namespace bayesfunc
{
   void evaluate_bayesian_network(
      int n_nodes,double tau,double node_time[],
      double apriori_maneuvering_prob[],
      double pstationary_given_evidence[],
      double pmaneuvering_given_evidence[],
      double manevuering_belief[]);
   void initialize_bayesian_network(
      int n_nodes,double tau,
      double node_time[],double apriori_maneuvering_prob[],
      NETNODE **status);
   void update_bayesian_network(
      int currnode,NETNODE **status,NETVECT *evidence_ptr,
      double pstationary_given_evidence,double pmaneuvering_given_evidence);
}

#endif // bayesian.h


