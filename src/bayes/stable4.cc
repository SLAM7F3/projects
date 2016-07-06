// =================================================================
// Program STABLE uses Tim Wallace's Bayesian Belief Network C code to
// compute non-maneuvering intervals for A2/AU satellites.
// =================================================================
// Last updated on 7/4/01
// =================================================================

extern "C"
{
   

#include <malloc.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>

#define EXTERNAL
#include "debug_wallace.h"
#include "bbndefs.h"
#include "bayesian.h"

// =================================================================
int main(int argc, char **argv)
{
   char name[80];
   int i,ntimesteps=9;
   double node_time[ntimesteps];
   NETNODE **status;
   NETVECT evidence;

   node_time[0]=0;
   node_time[1]=5;
   node_time[2]=7;
   node_time[3]=8;
   node_time[4]=22;
   node_time[5]=25;
   node_time[6]=27;
   node_time[7]=40;
   node_time[8]=42;
   
   status = (NETNODE **)calloc(ntimesteps,sizeof(NETNODE *));
   init_netvect(&evidence,2);

   initialize_bayesian_network(ntimesteps,node_time,status);

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

   for (i=0; i<ntimesteps; i++)
   {
      printf("i = %d  belief[0] = %f  belief[1] = %f \n",
             i,status[i]->belief.dat[0],status[i]->belief.dat[1]);
   }
}

}
