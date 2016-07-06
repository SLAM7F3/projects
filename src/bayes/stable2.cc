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

// =================================================================
int main(int argc, char **argv)
{
   char name[80];
   int i,ntimesteps=9;
   NETNODE *clouds, *rain, *sunburn, *play_game;
   NETVECT rain_alarm, phone_call;
   NETNODE **status;
   NETVECT evidence;

   status = (NETNODE **)calloc(ntimesteps,sizeof(NETNODE *));

// Initialize root node:

   status[0] = create_node(2,0,"0");
   add_statename(status[0],0,"stationary");
   add_statename(status[0],1,"maneuvering");
   status[0]->pi.dat[0]=0.8;
   status[0]->pi.dat[1]=0.2;
   revise_belief(status[0]);
 
// Initialize remaining nodes within chain:

   for (i=1; i<ntimesteps; i++)
   {
      sprintf(name,"%d",i);
      status[i] = create_node(2,2,name);
      add_statename(status[i],0,"stationary");
      add_statename(status[i],1,"maneuvering");
      status[i]->pi.dat[0]=0.8;
      status[i]->pi.dat[1]=0.2;

      status[i-1]->first_child=status[i];
      status[i]->parent=status[i-1];
      nvcopy(&(status[i]->incoming_pi),&(status[i-1]->belief));
      status[i]->CP.mat[0][0] = 0.9;
      status[i]->CP.mat[0][1] = 0.1; 
      status[i]->CP.mat[1][0] = 0.1; 
      status[i]->CP.mat[1][1] = 0.9;
      build_transpose(status[i]);
      revise_belief(status[i]);
   }
   
   dump_network(status[0]);

// Now have network in initial state, reflecting prior beliefs only
// Need to admit evidence, and then update network.

   fprintf(stderr,"\n******* Adding evidence *******\n");

   init_netvect(&evidence,2);
   evidence.dat[0] = 0.75; // chance of stationary given cross range profile 
   evidence.dat[1] = 0.25; // chance of maneuver given cross range profile  
   submit_evidence(status[0],&evidence);

   evidence.dat[0] = 0.65; // chance of stationary given cross range profile 
   evidence.dat[1] = 0.35; // chance of maneuver given cross range profile  
   submit_evidence(status[1],&evidence);

   evidence.dat[0] = 0.15; // chance of stationary given cross range profile 
   evidence.dat[1] = 0.85; // chance of maneuver given cross range profile  
   submit_evidence(status[2],&evidence);

   evidence.dat[0] = 0.85; // chance of stationary given cross range profile 
   evidence.dat[1] = 0.15; // chance of maneuver given cross range profile  
   submit_evidence(status[3],&evidence);

   evidence.dat[0] = 0.75; // chance of stationary given cross range profile 
   evidence.dat[1] = 0.25; // chance of maneuver given cross range profile  
   submit_evidence(status[4],&evidence);

   evidence.dat[0] = 0.25; // chance of stationary given cross range profile 
   evidence.dat[1] = 0.75; // chance of maneuver given cross range profile  
   submit_evidence(status[5],&evidence);

   evidence.dat[0] = 0.20; // chance of stationary given cross range profile 
   evidence.dat[1] = 0.80; // chance of maneuver given cross range profile  
   submit_evidence(status[6],&evidence);

   evidence.dat[0] = 0.55; // chance of stationary given cross range profile 
   evidence.dat[1] = 0.45; // chance of maneuver given cross range profile  
   submit_evidence(status[7],&evidence);

   evidence.dat[0] = 0.15; // chance of stationary given cross range profile 
   evidence.dat[1] = 0.85; // chance of maneuver given cross range profile  
   submit_evidence(status[8],&evidence);

   dump_network(status[0]);
}

