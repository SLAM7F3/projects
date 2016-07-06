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
   int i,ntimesteps=20;
   NETNODE *clouds, *rain, *sunburn, *play_game;
   NETVECT rain_alarm, phone_call;
   NETNODE **status;

   status = (NETNODE **)calloc(ntimesteps,sizeof(NETNODE *));

// Initialize root node:

   status[0] = create_node(2,0,"timestep0");
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
      status[i]->CP.mat[0][0] = 0.8;
      status[i]->CP.mat[0][1] = 0.2; 
      status[i]->CP.mat[1][0] = 0.2; 
      status[i]->CP.mat[1][1] = 0.8;
      build_transpose(status[i]);
      revise_belief(status[i]);
   }
   

   /* clouds are the root node; nothing in our system causes clouds */
   clouds = create_node(2,0,"Clouds");
   add_statename(clouds,0,"cloudy");
   add_statename(clouds,1,"clear");
   /* apriori probability of clouds is 10% */
   clouds->pi.dat[0] = 0.1;
   clouds->pi.dat[1] = 0.9;

   if (!revise_belief(clouds)) exit(2);
   /* with no external evidence or children, belief is now 10% also */

   rain = create_node(2,2,"Rain");
   add_statename(rain,0,"rain");
   add_statename(rain,1,"dry");

   /* clouds are a cause of rain */
   clouds->first_child = rain;
   rain->parent = clouds;
   nvcopy(&(rain->incoming_pi),&(clouds->belief));

   rain->CP.mat[0][0] = 0.6; /* chance of rain if clouds */
   rain->CP.mat[0][1] = 0.4; /* chance of no rain if clouds */
   rain->CP.mat[1][0] = 0.0; /* chance of rain if clear */
   rain->CP.mat[1][1] = 1.0; /* chance of no rain if clear */
   build_transpose(rain);
 
   /* propagate incoming pi from clouds to update rain belief */
   if (!revise_belief(rain)) exit(3);

   play_game = create_node(2,2,"Play Game");
   add_statename(play_game,0,"play game");
   add_statename(play_game,1,"postpone");

   /* play_game is affected by rain */
   rain->first_child = play_game;
   play_game->parent = rain;
   nvcopy(&(play_game->incoming_pi),&(rain->belief));

   play_game->CP.mat[0][0] = 0.05; /* chance of play game if rain */
   play_game->CP.mat[0][1] = 0.95; /* chance of postpone if rain */
   play_game->CP.mat[1][0] = 1.0; /*  chance of play game if dry */
   play_game->CP.mat[1][1] = 0.0; /*  chance of postpone if dry */
   build_transpose(play_game);

   /* propagate incoming belief from rain to update play_game */
   if (!revise_belief(play_game)) exit(4);

   sunburn = create_node(2,2,"Sunburn");
   add_statename(sunburn,0,"sunburn");
   add_statename(sunburn,1,"no sunburn");
   /* sunburn is affected by clouds.  Since clouds already has a
      first child (rain), use next_sibling */
   rain->next_sibling = sunburn;
   sunburn->parent = clouds;
   nvcopy(&(sunburn->incoming_pi),&(clouds->belief));

   sunburn->CP.mat[0][0] = 0.1; /* chance of sunburn if clouds */
   sunburn->CP.mat[0][1] = 0.9; /* chance of no sunburn if clouds */
   sunburn->CP.mat[1][0] = 0.7; /* chance of sunburn if clear */
   sunburn->CP.mat[1][1] = 0.3; /* chance of no sunburn if clear */
   build_transpose(sunburn);

   /* propagate incoming belief from clouds to update sunburn */
   if (!revise_belief(sunburn)) exit(5);

/* Now have network in initial state, reflecting prior beliefs only
   Need to admit evidence, and then update network.  First, assume
   get evidence on rain from rain alarm, lambda (0.8,0.04)
*/
   dump_network(clouds);

   fprintf(stderr,"\n******* Adding evidence from rain detector *******\n");

   init_netvect(&rain_alarm,2);
   rain_alarm.dat[0] = 0.8; /* chance of rain given alarm */
   rain_alarm.dat[1] = 0.04; /* chance of no rain given alarm */
   if (!submit_evidence(rain,&rain_alarm)) exit(6);

   dump_network(clouds);

/* Now add evidence from phone call on sunburn.  Likelihoods are 1, .02 */
   fprintf(stderr,"\n******* Adding evidence from phone call *******\n");

   init_netvect(&phone_call,2);
   phone_call.dat[0] = 1.0; /* chance of sunburn given phone call */
   phone_call.dat[1] = 0.02; /* chance of no sunburn given phone call */
   if (!submit_evidence(sunburn,&phone_call)) exit(7);
  
   dump_network(clouds);
}

