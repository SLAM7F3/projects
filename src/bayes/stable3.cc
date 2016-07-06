// =================================================================
// Program STABLE uses Tim Wallace's Bayesian Belief Network C code to
// compute non-maneuvering intervals for A2/AU satellites.
// =================================================================
// Last updated on 7/3/01
// =================================================================

#include <malloc.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include "bbndefs.h"

#define EXTERNAL
#include "debug_wallace.h"
#include "bayesian.h"

// =================================================================
int main(int argc, char **argv)
{
   char name[80];
   int i,ntimesteps=9;
   NETNODE **status;
   NETVECT evidence;

   status = (NETNODE **)calloc(ntimesteps,sizeof(NETNODE *));
   init_netvect(&evidence,2);

   initialize_bayesian_network(ntimesteps,status);

// Now have network in initial state, reflecting prior beliefs only
// Need to admit evidence, and then update network.

   fprintf(stderr,"\n******* Adding evidence *******\n");

   update_bayesian_network(0,status,&evidence,0.75,0.25);
   update_bayesian_network(1,status,&evidence,0.65,0.35);
   update_bayesian_network(2,status,&evidence,0.15,0.85);
   update_bayesian_network(3,status,&evidence,0.85,0.15);
   update_bayesian_network(4,status,&evidence,0.75,0.25);
   update_bayesian_network(5,status,&evidence,0.25,0.75);
   update_bayesian_network(6,status,&evidence,0.20,0.80);
   update_bayesian_network(7,status,&evidence,0.55,0.45);
   update_bayesian_network(8,status,&evidence,0.15,0.85);

   dump_network(status[0]);
}

