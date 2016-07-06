/* tstbbn.c: test routines for updating Bayesian belief networks
	using example from May '89 AI Expert, pp. 44-48
   T. Wallace, 5/2/00
*/

#include "bbndefs.h"

#define EXTERNAL
#include "debug_wallace.h"

#include <malloc.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>

#define PROG "tstbbn"

NETNODE *clouds, *rain, *sunburn, *play_game;
NETVECT rain_alarm, phone_call;

void syntax(int exitval);

extern char *optarg;
extern int optind;

int main(int argc, char **argv)
{
  int ch;

  debug = 0;
  while ((ch=getopt(argc,argv,"d:hH"))!=EOF) {
    switch (ch) {
#ifdef DEBUG
	case 'd':
	  debug = atoi(optarg);
	  break;
#endif
	case 'H':
	case 'h':
	case '?':
		syntax(1);
    }
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

  return 0;
}

void syntax(int exitval)
{
	fprintf(stderr,"Syntax: %s [",PROG);
	fprintf(stderr," -h (thismsg)");
#ifdef DEBUG
	fprintf(stderr," -ddebugflag");
#endif
	fprintf(stderr," ]\n");
	exit(exitval);
}
