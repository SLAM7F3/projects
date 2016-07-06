/* satbbn.c: routines for setting up Bayesian belief networks
	representing GEO satellite status/measurement status
	Should re-think numbers for LEO, although code will work fine.
   T. Wallace, 5/8/00
*/

#include "bbndefs.h"

/* TESTING means running this as a stand-alone program with main,
rather than just as a library of functions which produce matrices.
It basically includes main() and syntax(), and their necessary stuff.
*/

#ifndef TESTING
#define TESTING
#endif

#ifdef TESTING
#define EXTERNAL 
#else
#define EXTERNAL extern
#endif
#include "debug_wallace.h"

#include <malloc.h>
#include <stdio.h>
#include <string.h>

#ifdef TESTING
#include <getopt.h>
#include <stdlib.h>
#include <math.h>
#define PROG "satbbn"
#define LIFETIME 3.0
#endif

#define SBVQUALITY 0.95
#define DAYS_PER_YR 365.25
#define NSTATUS_STATES 2
#define NDATA_STATES 3

double initial_prob_good(double days,double lifetime_yrs);
int build_cpstatus(NETNODE *n,double days_since_launch,
                   double parent_days_since_launch, double lifetime_yrs);
double prob_recover(double days_since_launch,
                    double parent_days_since_launch, double lifetime_yrs);
int build_cpdata(NETNODE *n, double sensor_qual);
void conf_evid(double conf, NETVECT *evid);
NETNODE *create_stat_node(const char *name, NETNODE *parent);
NETNODE *create_data_node(const char *name, NETNODE *pnode);
double prob_failure(double days_since_launch,
                    double parent_days_since_launch, double lifetime_yrs);
double failure_rate(double lifetime_fract);
double loglike_conf(double loglike,double min,double max);
int sat_bbn(double *input_evid, double *input_daynum, int count,
            NETNODE **stat, NETNODE **dstat, double initial_prob,
            double lifetime_years);
int bl_sat_bbn(double *input_evid, double *input_daynum, int count,
               double initial_prob, double lifetime_years, int blksize,
               int overlap, double *statvect, double *sigvect);

#ifdef TESTING

void syntax(int exitval);

extern char *optarg;
extern int optind;

#define MAXSIGS 5000
double input_evid[MAXSIGS];
double input_daynum[MAXSIGS];
int extra_num[MAXSIGS];

int main(int argc, char **argv)
{
   int ch;
   double loglike;
   char buf[100];
   int i, count, nfields;
   double *statvect, *sigvect;
   int nsigs = MAXSIGS;
 
/* satellite lifetime, years */
   double lifetime_years = LIFETIME;
/* offset between day numbers read and actual launch day */
   double launch_offset_days = 0.0;
   double initial_prob = -1.0;

   int sig_node = 0; /* include sig node output */
   int stat_node = 0; /* include status node output */
   int input_conf = 0; /* include input evidence in output, as confidence */
   int input_days = 0; /* include input datetime in output, replacing index */
   int extra_stuff = 0; /* copy third integer from input to output */
   int blksize = 999999;	/* block size if breaking up net */
   int overlap = 0;	/* overlap size if breaking up net */

#ifdef DEBUG
   debug = 0;
#endif
  
   while ((ch=getopt(argc,argv,"b:d:ehHil:n:o:p:sSty:"))!=EOF) {
      switch (ch) {
         case 'b':
            blksize = atoi(optarg);
            break;
#ifdef DEBUG
         case 'd':
            debug = atoi(optarg);
            break;
#endif
         case 'e':
            extra_stuff = 1;
            break;
         case 'H':
         case 'h':
         case '?':
            syntax(1);
            break;
         case 'i':
            input_conf = 1;
            break;
         case 'l':
            launch_offset_days = atof(optarg);
            break;
         case 'n':
            nsigs = atoi(optarg);
            if (nsigs > MAXSIGS) {
               fprintf(stderr,"Warning, nsigs of %d > %d, using %d\n",
                       nsigs,MAXSIGS,MAXSIGS);
               nsigs = MAXSIGS;
            }
            break;
         case 'o':
            overlap = atoi(optarg);
            break;
         case 'p':
            initial_prob = atof(optarg);
            break;
         case 's':
            sig_node = 1;
            break;
         case 'S':
            stat_node = 1;
            break;
         case 't':
            input_days = 1;
            break;
         case 'y':
            lifetime_years = atof(optarg);
            break;
      }
   }

   count = 0;
   for (i=0; (fgets(buf,100,stdin) != NULL) && (i<nsigs); i++) {
      if ((nfields = sscanf(buf,"%lf %lf %d",&input_daynum[i],&loglike,
                            &extra_num[i])) < 2) {
         break;
      }
      input_evid[i] = loglike_conf(loglike,0.0,1.0);
      input_daynum[i] += launch_offset_days;
      TRACE(5,(DF,"Read: %d %g %g\n",i,input_daynum[i],input_evid[i]));
      count++;
   }
   if ((statvect = (double *)malloc(sizeof(double) * count)) == NULL) {
      exit(2);
   }
   if ((sigvect = (double *)malloc(sizeof(double) * count)) == NULL) {
      exit(3);
   }
   if (!bl_sat_bbn(input_evid,input_daynum,count,initial_prob,
                   lifetime_years,blksize,overlap,statvect,sigvect)) {
      exit(4);
   }
/* If no output options set, don't output anything */
   if (input_conf || stat_node || sig_node) {
      for (i=0; i<count; i++) {
         if (input_days) {
            printf("%.5f",input_daynum[i]);
         } else {
            printf("%d",i+1);
         }
         if (input_conf) printf(" %.4f",input_evid[i]);
         if (stat_node) printf(" %.4f",statvect[i]);
         if (sig_node) printf(" %.4f",sigvect[i]);
         if (extra_stuff) printf(" %d",extra_num[i]);
         printf("\n");
      }
   }
   return 0;
}

void syntax(int exitval)
{
   fprintf(stderr,"Syntax: %s [",PROG);
   fprintf(stderr," -h (thismsg)");
#ifdef DEBUG
   fprintf(stderr," -ddebugflag");
#endif
   fprintf(stderr," -n max_sigs_to_process");
   fprintf(stderr,"\n\t\t");
   fprintf(stderr," -l launch_offset_days (added to days read)");
   fprintf(stderr,"\n\t\t");
   fprintf(stderr," -y satellite_lifetime_years (default %.1f)",LIFETIME);
   fprintf(stderr,"\n\t\t");
   fprintf(stderr," -p initial_prob_stable (otherwise estimated)");
   fprintf(stderr,"\n\t\t");
   fprintf(stderr," -t (output input datetime, not index)");
   fprintf(stderr,"\n\t\t");
   fprintf(stderr," -i (include input confidences in output)");
   fprintf(stderr,"\n\t\t");
   fprintf(stderr," -S (include status node output)");
   fprintf(stderr,"\n\t\t");
   fprintf(stderr," -s (include sig node output)");
   fprintf(stderr,"\n\t\t");
   fprintf(stderr," -e (include extra integer from input file in output)");
   fprintf(stderr,"\n\t\t");
   fprintf(stderr," -b net_block_size (divide into smaller nets)");
   fprintf(stderr,"\n\t\t");
   fprintf(stderr," -o net_block_overlap (if -b used)");
   fprintf(stderr," ]\n\t");
   fprintf(stderr,"Input is two numbers per line, representing daynum, log_like\n");
   fprintf(stderr,"A log_like <= -100 causes data node to be omitted\n");
   exit(exitval);
}
#endif

int bl_sat_bbn(double *input_evid, double *input_daynum, int count,
               double initial_prob, double lifetime_years, int blksize,
               int overlap, double *statvect, double *sigvect)
{ /* given input evidence vector (log likelihood) and input_daynum vect
     (in decimal days since launch), both of dimension count,  build
     Bayesian net, returning stat/sig probs in [stat/sig]vect.  initial_prob
     is the initial prob to use: if <= 0, estimate using % of lifetime
     used.  lifetime_years is the typical satellite lifetime in years
     blksize is the size of the subnets used, and overlap is the overlap.
     The sizes are measured in time steps (2 BBN nodes per step).
     Returns true if successful
  */
   NETNODE **stat, **dstat;
   int i, success, place, nleft;

   /* let's call calloc as a potential aid to garbage collection if
      sat_bbn fails.  It's not guaranteed to work, I'm afraid, since
      I haven't really verified that every path out of sat_bbn leaves
      the network in a garbage-collectable state, but I think it does.
      (The careful code in create_node I think will do it)
   */
   if ((stat = (NETNODE **)calloc(count,sizeof(NETNODE *))) == NULL) {
      return 0;
   }
   if ((dstat = (NETNODE **)calloc(count,sizeof(NETNODE *))) == NULL) {
      free(stat);
      return 0;
   }
   place = 0;
   while (place < count) {
      nleft = count - place;
      if (nleft < blksize) {
         blksize = nleft;
         overlap = 0;
      }
      if ( (success = sat_bbn(&input_evid[place],&input_daynum[place],blksize,
                              stat,dstat,initial_prob,lifetime_years) ) ) {
         for (i=0; i<(blksize-overlap); i++) {
            statvect[place+i] = stat[i]->belief.dat[0];
            if (input_evid[place+i] >= 0.0)
               sigvect[place+i] = (input_evid[place+i] < 0.0) ? -1.0 :
               dstat[i]->belief.dat[0];
         }
         initial_prob = stat[blksize - overlap - 1]->pi.dat[0];
#ifdef DEBUG
         if (debug > 0) dump_network(stat[0]);
#endif
      }
      /* could change the code so that we can build a network out of
         existing nodes; would save a lot of malloc/free stuff */
      for (i=blksize-1; i>= 0; i--) {
/* Now that we may or may not have dstat nodes, need to explicitly set
   the dstat pointers to NNULL when freed or we may try to free them twice
*/
         if (dstat[i] != NNULL) {
            free_node(dstat[i]);
            dstat[i] = NNULL;
         }
         if (stat[i] != NNULL) {
            free_node(stat[i]);
            stat[i] = NNULL;
         }
      }
      /* if this one failed, go no further */
      if (!success) break;
      place += (blksize - overlap);
   }
   free(stat);
   free(dstat);
   return success;
}

int sat_bbn(double *input_evid, double *input_daynum, int count,
            NETNODE **stat, NETNODE **dstat, double initial_prob,
            double lifetime_years)
{ /* given input evidence vector (log likelihood) and input_daynum vect
     (in decimal days since launch), both of dimension count,  build
     Bayesian net, saving node addresses in stat/dstat.  initial_prob
     is the initial prob to use: if <= 0, estimate using % of lifetime
     used.  lifetime_years is the typical satellite lifetime in years

     Key situation is where we want a status node but there is no
     evidence (denoted by input_evid value < 0).  Here we omit the
     data node entirely, changing the structure of the network...

     Returns true if successful
  */
   NETVECT evid;
   int i;
   double lastdays;
   char name[80];

   TRACE(1,(DF,"sat_bbn %g %g %d . . %g %g\n",*input_evid,*input_daynum,
            count,initial_prob,lifetime_years));

   if ((stat[0] = create_stat_node("Root",NNULL)) == NNULL) return 0;
/* set apriori probability satellite is stable (new launch) */
   stat[0]->pi.dat[0] = (initial_prob > 0.0) ?
      initial_prob : initial_prob_good(input_daynum[0],lifetime_years);
   stat[0]->pi.dat[1] = 1.0 - stat[0]->pi.dat[0];
   if (!revise_belief(stat[0])) return 0;

/* the status of the satellite is a cause of the signature state
   Skip this node if have no evidence at this time */
   if (input_evid[0] >= 0.0) {
      if ((dstat[0] = create_data_node("d_Root",stat[0])) == NNULL) return 0;
      stat[0]->first_child = dstat[0];
      build_cpdata(dstat[0],SBVQUALITY);
      nvcopy(&(dstat[0]->incoming_pi),&(stat[0]->belief));
      if (!revise_belief(dstat[0])) return 0;
   }

/* now 1- or 2-node network is in initial state, prior to evidence */
#if 0
   dump_network(stat[0]);
   fprintf(stderr,"\n********** Now submitting sig conf evidence... ****\n");
#endif

   init_netvect(&evid,3);
/* submit evidence for 0th node, if we have any.  If not, skip node
 */
   if (input_evid[0] >= 0.0) {
      conf_evid(input_evid[0],&evid);
      if (!submit_evidence(dstat[0],&evid)) { free_netvect(&evid); return 0; }
   } else {
      TRACE(10,(DF,"Skipping update of data node 0 (no evidence)\n"));
   }

/* now do the main loop... */

   lastdays = input_daynum[0];
   for (i=1; i<count; i++) {
      sprintf(name,"%d",i);
/* the parent of this guy is the previous sat stat node */
      if ((stat[i] = create_stat_node(name,stat[i-1])) == NNULL)
      { free_netvect(&evid); return 0; }
/* If the previous time had evidence, then the parent already has one child,
   so need to use next_sibling field.  Otherwise, just use first_child */
      if (input_evid[i-1] >= 0.0) {
         dstat[i-1]->next_sibling = stat[i];
      } else {
         stat[i-1]->first_child = stat[i];
      }
      nvcopy(&(stat[i]->incoming_pi),&(stat[i-1]->belief));
      build_cpstatus(stat[i],input_daynum[i],lastdays,lifetime_years);
      if (!revise_belief(stat[i]))
      { free_netvect(&evid); return 0; }

      sprintf(name,"d%d",i);
/* the status of the satellite is a cause of the signature state,
   that is, if we have signature evidence:
*/
      if (input_evid[i] >= 0.0) {
         if ((dstat[i] = create_data_node(name,stat[i])) == NNULL) {
            free_netvect(&evid); return 0;
         }
         stat[i]->first_child = dstat[i];
         build_cpdata(dstat[i],SBVQUALITY);
         nvcopy(&(dstat[i]->incoming_pi),&(stat[i]->belief));
         if (!revise_belief(dstat[i])) { free_netvect(&evid); return 0; }
         conf_evid(input_evid[i],&evid);
         if (!submit_evidence(dstat[i],&evid)) {
            free_netvect(&evid); return 0;
         }
      } else {
         TRACE(10,(DF,"Skipping data node %d (no evidence)\n",i));
      }
      lastdays = input_daynum[i];
   }
   free_netvect(&evid);
   return 1;
}

double initial_prob_good(double days,double lifetime_yrs)
{ /* gives initial probability{good} to start things off.
     We assume a good launch should be at least .8, staying
     at .7 through the first half of the lifetime, dropping
     to .6 from 50 to 100% of the lifetime, and then to 0.5
     up to 2.0 times the lifetime, which isn't that unusual.
     For real old birds, odds are much less, say 0.2, 0.1
  */
   double fract;
   fract = days / (lifetime_yrs * DAYS_PER_YR);
   if (fract < 0.1) {
      return 0.8;
   } else if (fract < 0.5) {
      return 0.7;
   } else if (fract < 1.0) {
      return 0.6;
   } else if (fract < 2.0) {
      return 0.5;
   } else if (fract < 3.0) {
      return 0.2;
   } else {
      return 0.1;
   }
}

NETNODE *create_stat_node(const char *name, NETNODE *pnode)
{
   NETNODE *n;
   int psize;

/* If this is the root node, must call create node with parent dim 0 */
   psize = (pnode == NNULL) ? 0 :2;
   if ((n = create_node(2,psize,name)) == NNULL) return NNULL;
   add_statename(n,0,"stable");
   add_statename(n,1,"unstable");
   n->parent = pnode;
   TRACE(100,(DF,"create_stat_node created %s at %x\n",name,(unsigned int)n));
   return n;
}

NETNODE *create_data_node(const char *name, NETNODE *pnode)
{
   NETNODE *n;

   if ((n = create_node(3,2,name)) == NNULL) return NNULL;
   add_statename(n,0,"stable");
   add_statename(n,1,"unstable");
   add_statename(n,2,"mistag");
   n->parent = pnode;
   TRACE(100,(DF,"create_data_node created %s at %x\n",name,(unsigned int)n));
   return n;
}

int build_cpstatus(NETNODE *n,double days_since_launch,
                   double parent_days_since_launch, double lifetime_yrs)
{
/* build conditional probability matrix relating current status node *n
   to its parent, describing the satellite at time parent_days_since_launch.
   given that the expected lifetime of the satellite is lifetime_yrs years,
   and it has been days_since_launch since satellite was launched.

   We have two states for both parent and child: (stable, unstable)
   CP[i][j] is P{ child[j] | parent[i] }
*/
   double p_fail, p_recover;

   if (n->len != NSTATUS_STATES) {
      fprintf(stderr,"Bad NETNODE: len must be %d\n",NSTATUS_STATES);
      return 0;
   }
   if (n->CP.nrows != NSTATUS_STATES) {
      fprintf(stderr,"Bad NETNODE: nrows must be %d\n",NSTATUS_STATES);
      return 0;
   }

   p_fail = prob_failure(days_since_launch,parent_days_since_launch,
                         lifetime_yrs);

   n->CP.mat[0][0] = 1.0 - p_fail;
   n->CP.mat[0][1] = p_fail;
  
   p_recover = prob_recover(days_since_launch,parent_days_since_launch,
                            lifetime_yrs);
   n->CP.mat[1][0] = p_recover;
   n->CP.mat[1][1] = 1.0 - p_recover;
   build_transpose(n);
   return 1;
}

double prob_failure(double days_since_launch,
                    double parent_days_since_launch, double lifetime_yrs)
{ /*
    returns probability of satellite failure over interval
    (days_since_launch - parent_days_since_launch), given
    an expected lifetime of lifetime_yrs years.
  */
   double prob;
   double lifetime_days, gap_days;
   double lifetime_fract, parent_fract;
   double start_fract, fini_fract, p_reasonable;

/*	assume that probability of failure is 60%/year for first 60 days,
	(checkout time to cover birth defects).  We then revert to an
	underlying model in which p{fail} starts at 20%/year at launch,
	and increases linearly to 50%/year at end of lifetime.
	By failure we mean nothing more or less than loss of stability.
*/
   lifetime_days = lifetime_yrs * DAYS_PER_YR;
   lifetime_fract = days_since_launch / lifetime_days;
   gap_days = days_since_launch - parent_days_since_launch;
   prob = 0.0;
   if (parent_days_since_launch < 60) {
      /* have a birth-defect increment to add first... */
      if (days_since_launch < 60) {
         /*we're entirely in the first 60 days.  simple calc. */
         prob = (0.6 / DAYS_PER_YR) * gap_days;
         parent_days_since_launch = days_since_launch;
      } else {
         prob = (0.6 / DAYS_PER_YR) * (60 - parent_days_since_launch);
         parent_days_since_launch = 60.0;
      }
   }
   parent_fract = parent_days_since_launch / lifetime_days;

   if (parent_days_since_launch < days_since_launch) {
/*	Add in the area under the ramp for
        prob{failed since birth-defect epoch}
*/
      start_fract = failure_rate(parent_fract);
      fini_fract = failure_rate(lifetime_fract);
      prob += (( start_fract + fini_fract) / 2.0) * (gap_days / DAYS_PER_YR);
   }
/*
  This is all well and good for short intervals, but need to
  impose limits in case we have a really big gap.  Can't have
  probability of failure > 1.  To see what max failure prob might
  be reasonable, consult routine initial_prob_good.  Right now,
  that should limit p_fail to 90%.
*/
   p_reasonable = 1.0 - initial_prob_good(days_since_launch,lifetime_yrs);
   if (prob > p_reasonable) prob = p_reasonable;
   return prob;
}

double failure_rate(double lifetime_fract)
{ /*	gives failure rate per year for a satellite at
	``lifetime_fract'' of its total expected lifetime.
  */
   double prob;
   prob = 0.20 + lifetime_fract * .30;
   if (prob > 0.9) prob = 0.9;
   return prob;
}

double prob_recover(double days_since_launch,
                    double parent_days_since_launch, double lifetime_yrs)
{ /* simple model of prob { satellite will be recovered in interval
     defined by first two args}
     Many scenarios come to mind.  To simplify, what if we argue
     that for young satellites (less than half lifetime, say),
     the probability is 2% over a day, dropping to 0.2% for
     satellites past their expected lifetime.  Again need
     to implement max vals, though; otherwise if have no data for
     long time, might get prob{recovery} > 1!
  */
/* YOUNG is measured in fractions of satellite lifetime, YOUNG_RECOVER
   is prob of recovery in a day for sat < ``YOUNG fraction'' old */
#define YOUNG 0.25
#define YOUNG_RECOVER 0.02
#define OLD 1.0
#define OLD_RECOVER 0.002

   double prob, daily_rate, lifetime_days;
   double lifetime_fract;
   double p_recoverable;

   lifetime_days = lifetime_yrs * DAYS_PER_YR;
   lifetime_fract = days_since_launch / lifetime_days;
   if (lifetime_fract <= YOUNG) {
      daily_rate = YOUNG_RECOVER;
   } else if (lifetime_fract >= OLD) {
      daily_rate = OLD_RECOVER;
   } else { /* linear interpolation between above two */
      daily_rate = YOUNG_RECOVER - ( (lifetime_fract - YOUNG) / (OLD - YOUNG) )
	 * (YOUNG_RECOVER - OLD_RECOVER);
   }
/* What should the limit on this stuff be?  Right now, the bigger
   the gap between passes, the higher the p{recover}, right beyond
   1.0.  Figure out reasonable number for prob sat could be in a
   recoverable state, then divide by something like 50...
*/
   prob = daily_rate * (days_since_launch - parent_days_since_launch);
   p_recoverable = initial_prob_good(days_since_launch,lifetime_yrs);
   if (prob > (p_recoverable/10)) prob = p_recoverable / 10;
   TRACE(10,(DF,"prob_recover: rate=%.3f prob=%.3f max=%.3f\n",daily_rate,prob,p_recoverable));
   return prob;
}

int build_cpdata(NETNODE *n, double sensor_qual)
{
/* build conditional probability matrix relating current data node *n
   to its parent status node, given that the sensor quality is sensor_qual
   The sensor quality should be a number between 0 and 1.0, higher better.

   We have two states for the parent status node: (stable, unstable)
   We have three states for the data node: (stable, unstable, mistag)
   a) data represents stable satellite
   b) data represents unstable satellite
   c) data represents mistag
   Note the difference between the data representing a stable satellite
   (which can be true even if it's not stable, and is the data node state)
   and the satellite actually being stable (the status node state)

   CP[i][j] is P{ child[j] | parent[i] }
*/
   double p_correct;
   double mistag = 0.05;
 
  
   if (n->CP.nrows != NSTATUS_STATES) {
      fprintf(stderr,"Bad NETNODE: len must be %d\n",NSTATUS_STATES);
      return 0;
   }
   if (n->len != NDATA_STATES) {
      fprintf(stderr,"Bad NETNODE: len must be %d\n",NDATA_STATES);
      return 0;
   }
   if (sensor_qual < 0.0 || sensor_qual > 1.0) {
      fprintf(stderr,"Bad sensor_quality: must be between 0 and 1.0\n");
      return 0;
   }
/*	Think of the sensor quality as the transfer factor from true status
	to sensor status.  There is no effect on the mistag belief from the
	true satellite status; this has to come from elsewhere.  We handle
	this by using the same value for mat[*][2], so odds are 1:1.
	This needs to be non-zero so PI sent down from status node doesn't
	always just zero out our mistag belief
*/
   p_correct = (1.0 - mistag) * sensor_qual;
   n->CP.mat[0][0] = p_correct;
   n->CP.mat[0][1] = 1.0 - p_correct - mistag;
   n->CP.mat[0][2] = mistag;
   n->CP.mat[1][0] = 1.0 - p_correct - mistag;
   n->CP.mat[1][1] = p_correct;
   n->CP.mat[1][2] = mistag;

   build_transpose(n);

   return 1;
}

void conf_evid(double conf, NETVECT *evid)
{ /* convert input confidence into evidence for sat data node
     just assume 5% probability of mistag at this point */

   evid->dat[0] = conf * 0.95;
   evid->dat[1] = (1.0 - conf) * 0.95;
   evid->dat[2] = 0.05;
   TRACE(5,(DF,"conf_evid: conf %g -> evid %.3f\n",conf,evid->dat[0]));
}

double loglike_conf(double loglike,double min,double max)
{ /* convert log likelihood to conf, forced to lie between given vals.
     A val <= -100 is a flag, meaning no evidence, and -1 is returned */

   double odds, conf;

   if (loglike <= -100.0) {
      conf = -1.0;
   } else {
      odds = exp(loglike);
      conf = odds / (odds + 1);
      if (conf < min) conf = min;
      if (conf > max) conf = max;
   }
   return conf;
}
