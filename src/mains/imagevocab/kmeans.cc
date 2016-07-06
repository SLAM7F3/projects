/* 
    Copyright (C) 2009,2010 Wei Dong <wdong.pku@gmail.com>. All Rights Reserved.

    DISTRIBUTION OF THIS PROGRAM IN EITHER BINARY OR SOURCE CODE FORM MUST BE
    PERMITTED BY THE AUTHOR.  NO COMMERCIAL USE ALLOWED.
*/

/*
   Introduction

   This program is a re-implementation of the k-means clustering
   algorithm.  It has the following features:

   1. It is an out-of-core implementation which allows you to cluster datasets
   larger than main memory.
   2. It supports parallel reading from multiple input files to maximize input
   throughput (when the input files are on independent devices).
   3. It is multi-threaded so you can use the full processing power of multiple
   processors (without MPI).
   4. With MPI, multiple machines can be used.
   5. It accelerates L2 distance calculation with BLAS or KD-tree.
*/

/* Compiling

   This program has been successfully compiled by me on a Linux
   platform with gcc, but you should be able to compile it on any unix
   platform supporting pthread with a standard C99 compiler with
   minimal modification.  I usually use the following command on my
   platform.

   gcc -Wall -O3 -g -std=c99 -o kmeans kmeans.c -lpthread -lm

   For simplicity and speed, some parameters are hard coded into this
   file, please go to the compile time parameter section to adjust
   these parameters.  You can override those parameters by defining
   them on the compiling commandline.  These parameters allow you to
   determine which optimization to use (BLAS or kd-tree), whether or
   not to do value checking and what text file field and line
   delimitors to use.  Note that the following two features require
   extra software support: 1. USE_MPI : you need an MPI platform.  I
   use mpich, but openmpi or others should also do.  2. USE_BLAS: you
   need a BLAS library.  I use ATLAS, but Intel MKL should be a better
   choice on Intel platforms.

   Note that USE_BLAS and USE_KD_TREE cannot be enabled together and
   by default none is enabled.  Following is a guideline to choose
   between BLAS and kd-tree or neither:

   * If you have a BLAS library, then USE_BLAS is always faster than using
   neither BLAS or kd-tree.
   * BLAS is faster than kd-tree when K is not very large or when the data
   dimension (D) is not very small.  For K < 10000 and D > 10, BLAS should
   be faster than kd-tree.
   * Kd-tree works best when D is small and K is very large.
   */

/* Input/Output File Format:
 
   This program is quite flexible on input file format.  It supports
   both binary (preferred) and text format.

   Binary Input:
   
   * The file can contain a header of any length (-h).
   * Each vector is stored as consecutive 32-bit floating point numbers with
   CPU endian.
   * Vectors are sequentially stored with a fixed gap (-g) between two consecutive
   vectors.  Note that the gap is also read into memory.
   * Use -s to specify the amount of data you want to read from each file.
   
   Text Input:

   * The file can contain a header of any fixed number of lines (-h).
   * Following that, each line contains a vector.  You can choose
   to skip a fixed number of fields from each line (-c).
   * The default line delimitor '\n' and field delimitor '\t' can be modified 
   in the "COMPILE TIME PARAMETERS" section of this file.

   Outputs are binary files with 0-header and 0-gap.
*/

/* Usage:
 
   Command line:
   kmeans [options] output input1 [input2...]
 
   1. Essential parameters
   The capitalized parameters are the essential parameters of the
   K-means algorithm, you need to provide a value for each of them.

   -K K    # clusters desired
   -I I    maximal # iterations
   -T T    converge threshold
   -D D    dimension of data

   -F  if empty clusters appear, select random vector
   as cluster centers
   -S S    use the initial cluster centers stored in file S    
   -P  partition the input dataset to clusters and write to files output-[0-(K-1)] instead
   of clustering. Automatically set I=1 and T=0 to ensure one and only one iteration is
   executed.  See note below.

   2. Performance parameters
   -w w    # working thread; best speed is achieved
   by setting w to the number of CPU cores you have
   -b b    process the data in blocks of size b
   the program might enlarge b a little bit so
   it is multiple of vector size
   can be something like "500, 500K, 500B, 1G"
   -d d    # of blocks to save in memory
        
   The program divide the dataset into sequence of blocks, and
   these blocks are processed by w threads.  

   3. Format parameters
   -t  by default, binary format is used; when -t is given
   text format is used
   -f f    all input values are checked for bad numbers
   by default, vectors with bad numbers are thrown
   away.  If f is given, then bad numbers are replaced
   with f and the vectors are used as if they are good
   ones.
   Binary format:
   -h h    h bytes are skipped at the head of each input file
   -g g    g bytes are skipped after each vector
   -s s    at most size s of each file is used, can be like
   "500, 500K, 500M, 2G"

   Text format:
   -h h    h lines are skipped at the head of each input file
   -c c    c fields are skipped at the head of each line
        
   4. MPI parameters
   -l  files are local to each node    
    
   5. Fault tolerance parameters
   -r r    when I/O error happens, retry r times before quitting
   the program

   -e  save the result at each loop

   6. Misc
   -v  show more logging information, can appear multiple times
   -q  show less logging information, can appear multiple times
   -p  # blocks used for generating initial clusters

   If initial clusters are not given via the -S parameter,the initial
   cluster centers will be sampled at random from the first p blocks
   read.  If p is not provided or if the p provided is too small, the
   least number of blocks that holds more than K vectors will be used.
   -z z output cluster sizes to file z


   The non-MPI program catches SIGTERM and SIGINT when clustering is
   going on.  When SIGTERM is received (killall kmeans), it will abort
   the current loop and writes the results of the previous loop; when
   SIGINT (type Ctrl-C in commandline), it will report the current
   status and continue working.  To kill the program immmediately, use
   killall -9 kmeans.

   Some extra note:

   * The program creates on reading thread for each input file.
   Splitting the input data will only improve throughput when they are
   stored on independent disk drives.  Multiple files on a single disk
   drive can only slow down reading.  * When using with MPI, if -l is
   specified, then the input files are treated as different files with
   the same names on different nodes.  If -l is not given, then the
   input files are treated as the same files on different nodes; the
   files are logically splitted and each node reads a different
   portion from each file.  * The parameter -P allows you to write
   data belonging to different cluster center to separate files.  It
   is intended to be used to partition the dataset according to
   existing clustering result with the following commandline:
            
   kmeans -S <previous-clustering-result> -P output input...

   If the program is interrupted, the integrity of the files are not
   guaranteed (while the program always tries to write the clustering
   result of the last finished iteration before quitting).  Since only
   one thread is used for writing the partitioned data, partitioning
   can be the bottleneck when dataset is large.
*/

/* Change History
 * 2010-02-08:
 *      Enable saving partitioned dataset(-P option) by adding
 *      a partitioner thread in the pipeline.
 *      L2 distance calculation acceleration with BLAS
 */

/**** COMPILE TIME PARAMETERS ****/

#define BACKUP(n)   ((n)/100+100)

#ifndef CHECK_NUMBER              // 1: to replace bad values with given value
#define CHECK_NUMBER    1         // 0: assume all input data have good values.
#endif

#ifndef USE_BLAS
#define USE_BLAS    1             // 1: use BLAS for fast L2 distance calculation
                                  // should not be used with KD_TREE
#endif

#ifndef USE_KD_TREE
#define USE_KD_TREE 0             // 1: use KD tree 0: use linear search
#endif

#ifndef VERIFY_KD_TREE           // test the kd-tree implementation, use 0
#define VERIFY_KD_TREE  0
#endif

#ifndef USE_MPI
#define USE_MPI 0
#endif

#define DEFAULT_MODE 0644

#define TXT_BUF_SIZE    100000   // enlarge this if your text input files have very long lines
#define TXT_BUF_SIZE_P1 (TXT_BUF_SIZE + 1)
#define TXT_LINE_DELIM  '\n'
#define TXT_FIELD_DELIM '\t'

/**** END OF COMPILE TIME PARAMETERS ****/

#if USE_KD_TREE
#if USE_BLAS
#error kdtree and blas should not be used together.
#endif
#endif


#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <values.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

#include <iostream>

#if USE_BLAS
#include <cblas.h>
#endif
#if USE_MPI
#include <mpi.h>
#endif

using std::cout;
using std::endl;

// global parameters

unsigned dim = 128;
unsigned K = 1000;          // # clusters
size_t vec_size;            // dim * sizeof(float)
size_t stride;              // VEC_SIZE + gap_size
#if USE_BLAS
int cblas_lda;
#endif
unsigned Kdim;              // K * dim
unsigned max_it = 100;      // max # iterations
float delta = 0.001;          // convergence threshold

float fill_value = 0;
size_t head_size = 0;       // skip this number of bytes for binary file
                            // or lines for text file at the beginning
unsigned skip_field = 0;    // skip this number of fields in text file
size_t gap_size = 0;        // number of bytes between vectors in binary file
size_t used_size = ULLONG_MAX; // size of each file used, binary file only
int txt_input = 0;
int skip_bad_number = 1;

unsigned n_reader;          // reading threads
unsigned n_worker = 1;      // # working threads
                            // there are at most 1 partitioner
unsigned n_partitioner = 0; // if -P is given, then there is one partitioner thread
                            // n_partitioner should not be > 1

unsigned n_buf_blk = 20;    // # buffer blocks allocated
size_t blk_sz = 0x100000;  // block size, must be x vec_size
// automatically adjusted.
size_t blk_vec;             // vector per block

int local_file = 0;
int is_root = 0;            // for non-MPI mode, always 1
int save_loop = 0;
// for MPI mode, is_root = (mpi_rank == 0)
int refill = 0;
int seeded = 0;

char **input_paths;         // part of argv
char seed_path[BUFSIZ];      
char size_path[BUFSIZ];      
const char *output_path;

unsigned n_backup;          // # backup vectors in case empty cluster appears
                            // = BACKUP when cluster seed provided
                            // or K + BACKUP otherwise
unsigned n_backup_node;
unsigned n_seed_vec;        // # vec needed to startup
unsigned n_hld_blk = 0;         // held blocks for initial centers
unsigned n_th;              // n_reader + n_worker
size_t total_blk;           // total # blocks in all files

#if USE_MPI
int mpi_size;
int mpi_rank;
#endif

int early_stop;             // set when we receive SIGTERM 

unsigned max_retry = 10;    // maximal number of retries at I/O error

enum { DEBUG = 0, LOCAL, INFO, WARN, ERROR, FATAL, SIGNAL }; // log levels
#if USE_MPI
int loglevel = INFO;
#else
int loglevel = LOCAL;
#endif

#define verify(_x)                                                      \
   do {                                                                 \
      if (!(_x)) {                                                      \
         LOG(FATAL, "Assertion failure in %s line %d of %s: %s\n",      \
             __FUNCTION__, __LINE__, __FILE__, #_x);                    \
      }                                                                 \
   } while (0)

void LOG(int level, const char *fmt, ...)
{
   static const char *level_str[] = {
      "DEBUG", "LOCAL", "INFO", "WARN", "ERROR", "FATAL", "SIGNAL"
   };
   assert(loglevel >= DEBUG);
   assert(loglevel <= SIGNAL);
   if (level >= loglevel) {
      va_list args;
      fprintf(stderr, "[%s] ", level_str[level]);
#if USE_MPI
      fprintf(stderr, "[MPI:%02d] ", mpi_rank);
#endif
      if (level == ERROR) {
         fprintf(stderr, "(errno: %s) ", strerror(errno));
      }
      va_start(args, fmt);
      vfprintf(stderr, fmt, args);
      va_end(args);
   }
   if (level == FATAL) {
#if USE_MPI
      MPI_Abort(MPI_COMM_WORLD, -1);
#else
      exit(-1);
#endif

   }
}

void stop (int signum);
void status (int signum);
void run (void);

void copyright (void) {
   fprintf(stderr, "The Out-Of-Core K-Means Clusterer\n");
   fprintf(stderr, "Copyright (C) 2009,2010 Wei Dong <wdong.pku@gmail.com>. All Rights Reserved.\n");
   fprintf(stderr, "Do not redistribute the program in either binary or source code form.\n");
}

void usage (void) {
   LOG(WARN, "Invalid command line.\n");
   printf("Usage: k-means clustering\n"
          "    kmeans [options] <out> <in...>\n\n"
          "Options:\n"
          "    -b b        block size (>1000)\n"
          "    -c c        number of fields to skip in each line for text input\n"
          "    -d d        number of blocks to allocate (>10)\n"
          "    -D dim      dimension of vector\n"
          "    -e          save results at each loop\n"
          "    -F          fill empty clusters with random vectors\n"
          "    -f f        replace bad value with f\n"
          "    -g g        bytes to skip between vectors\n"
          "    -h h        (binary input) bytes to skip in the beginning of file\n"
          "                (text input) lines to skip in the beginning of file\n"
          "    -I I        maximal iterations\n"
          "    -K K        number of clusters desired\n"
          "    -l          (only MPI) files are local\n"
          "    -p p        hold a pool of p blocks to produce initial centers\n"
          "                only used when <seed> is not given\n"
          "    -P          partition according to <seed> rather than clustering.\n"
          "                write partitioned dataset to out-[0-(K-1)]\n"
          "    -q          show less log messages\n"
          "    -r r        maximal I/O retry number\n"
          "    -S <seed>   initial cluster center\n"
          "    -s s        maximal size to read from each file\n"
          "    -t          use text input\n"
          "    -T T        convergence threshold\n"
          "    -v          show more log messages\n"
          "    -w w        number of working thread\n"
          "    -z z        output cluster size to z\n"
          "    <out>       output file\n"
          "    <in>        input files, can be '-' for STDIN\n"
          "\n"
      );
   exit(-1);
}

void show_params (void) {
   LOG(DEBUG, "Configuration:\n");
   LOG(DEBUG, "  DIM = %u\n", dim);
   LOG(DEBUG, "  K = %u\n", K);
   LOG(DEBUG, "  MAX IT = %u\n", max_it);
   LOG(DEBUG, "  DELTA = %g\n", delta);
   LOG(DEBUG, "  HEAD SIZE = %u\n", head_size);
   LOG(DEBUG, "  GAP_SIZE = %u\n", gap_size);
   LOG(DEBUG, "  MAX RETRY = %u\n", max_retry);
   LOG(DEBUG, "  READING TH = %u\n", n_reader);
   LOG(DEBUG, "  WORKING TH = %u\n", n_worker);
   LOG(DEBUG, "  BLOCK SIZE = %u\n", blk_sz);
   LOG(DEBUG, "  # BUF BLOCK = %u\n", n_buf_blk);
   LOG(DEBUG, "  REFILL = %d\n", refill);
   LOG(DEBUG, "  USER SEED = %d\n", seeded);
   LOG(DEBUG, "  TOTAL BLOCK = %d (0 if not available)\n", total_blk);
   LOG(DEBUG, "  TOTAL BACKUP VEC = %u\n", n_backup);
   LOG(DEBUG, "  BACKUP VEC = %u\n", n_backup_node);
   LOG(DEBUG, "  SEED POOL = %u\n", n_hld_blk);
}

size_t strtos (const char *str) {
   char *p = 0;
   size_t r = strtol(str, &p, 0);
   if (*p == 'k' || *p == 'K') {
      r *= 1024;
   }
   else if (*p == 'm' || *p == 'M') {
      r *= 1024 * 1024;
   }
   else if (*p == 'g' || *p == 'G') {
      r *= 1024 * 1024 * 1024;
   }
   return r;
}

int getopt(int argc, char * const argv[], const char *optstring);
extern char *optarg;
extern int optind, optopt, opterr;

int main (int argc, char *argv[]) {

   int c, r;

   if (sizeof(size_t) != sizeof(void *)) {
      fprintf(stderr, "size_t and void * are not of equal size.\n");
      return -1;
   }

#if USE_MPI
   r = MPI_Init(&argc, &argv);
   verify(r == MPI_SUCCESS);
   r = MPI_Comm_size(MPI_COMM_WORLD,&mpi_size);
   verify(r == MPI_SUCCESS);
   r = MPI_Comm_rank(MPI_COMM_WORLD,&mpi_rank);
   verify(r == MPI_SUCCESS);
    
   if (mpi_rank == 0) {
#endif

      is_root = 1;
      copyright();

#if USE_MPI
      LOG(INFO, "MPI initialized, size = %d.\n", mpi_size);
   }
#endif

   while ((c = getopt(argc, argv, "c:b:d:D:ef:Fg:h:I:K:lp:Pqr:s:S:tvT:w:z:")) != -1) {
      switch (c) {
         case 'c': skip_field = strtol(optarg, NULL, 0);
            break;
         case 'b': blk_sz = strtos(optarg);
            break;
         case 'd': n_buf_blk = strtol(optarg, NULL, 0);
            break;
         case 'D': dim = strtol(optarg, NULL, 0);
            break;
         case 'e': save_loop = 1;
            break;
         case 'f': fill_value = strtod(optarg, NULL);
            skip_bad_number = 0;
            break;
         case 'F': refill = 1;
            break;
         case 'g': gap_size = strtol(optarg, NULL, 0);
            break;
         case 'h': head_size = strtol(optarg, NULL, 0);
            break;
         case 'I': max_it = strtol(optarg, NULL, 0);
            break;
         case 'K': K = strtol(optarg, NULL, 0);
            break;
         case 'l': local_file = 1;
            break;
         case 'p': n_hld_blk = strtol(optarg, NULL, 0);
            break;
         case 'P': n_partitioner = 1;
            break;
         case 'q': if (loglevel < FATAL) loglevel++;
            break;
         case 'r': max_retry = strtol(optarg, NULL, 0);
            break;
         case 's': used_size = strtos(optarg);
            break;
         case 'S': strncpy(seed_path, optarg, BUFSIZ);
            break;
         case 't': txt_input = 1;
            break;
         case 'T': delta = strtod(optarg, NULL);
            break;
         case 'w': n_worker = strtol(optarg, NULL, 0);
            break;
         case 'v': if (loglevel > DEBUG) loglevel--;
            break;
         case 'z': strncpy(size_path, optarg, BUFSIZ);
            break;
         default:
            usage();

      }
   }

   if (txt_input == 1) {
#if USE_MPI
      if (!local_file) {
         LOG(FATAL, "Shared text input not supported in MPI mode.\n");
      }
#endif
      if (used_size < ULLONG_MAX) {
         LOG(FATAL, "User specified size not allowed with text input.\n");
      }

      if (gap_size > 0) {
         LOG(FATAL, "Gap size not needed for text input.\n");
      }
   }

   vec_size = dim * sizeof(float);
   stride = vec_size + gap_size;
#if USE_BLAS
   if (gap_size % sizeof(float) != 0) {
      LOG(FATAL, "gap_size %% sizeof(float) != 0, cannot use BLAS.\n");
   }
   cblas_lda = stride / sizeof(float);
#endif
   Kdim = K * dim;

   if (blk_sz < 1000) {
      blk_sz = 4096;
   }
   blk_vec = (blk_sz + stride - 1) / stride;
   blk_sz = blk_vec * stride;
   LOG(LOCAL, "Block size adjusted to %u.\n", blk_sz);

   n_backup_node = n_backup = n_seed_vec = 0;

   if (refill) {
#if USE_MPI
      n_backup_node = ((BACKUP(K) + n_worker - 1) / n_worker + mpi_size - 1) / mpi_size * n_worker;
      n_backup = n_backup_node * mpi_size;
#else
      n_backup = n_backup_node = (BACKUP(K) + n_worker - 1) / n_worker * n_worker;
#endif
      LOG(LOCAL, "Backup vector ajusted to %u.\n", n_backup);
      n_seed_vec = n_backup_node;
   }

   seeded = strlen(seed_path) > 0;
   if (is_root && !seeded) {
      n_seed_vec += K;
   }

   if (n_partitioner && !seeded) {
      LOG(FATAL, "partitioning requires a seed file (-S).");
   }


   if (blk_vec * n_hld_blk < n_seed_vec) {
      n_hld_blk = (n_seed_vec * stride + blk_sz - 1)/ blk_sz;
      LOG(LOCAL, "Holding block increased to %u.\n", n_hld_blk);
   }

   if (n_buf_blk < 10 || n_buf_blk < n_hld_blk) {
      n_buf_blk = 10 > n_hld_blk ? 10 : n_hld_blk;
      LOG(LOCAL, "Buffer block extended to %u.\n", n_buf_blk);
   }

   if (argc - optind < 2) {
      usage();
   }

   output_path = argv[optind++];

   n_reader = argc - optind;
   input_paths = argv + optind;

   n_th = n_reader + n_worker + n_partitioner;

   total_blk = 0;
#if USE_MPI
   if (is_root) {
      LOG(WARN, "Progress will not be reported in MPI mode.\n");
      LOG(WARN, "Signals will not be caught in MPI mode (^C immediately kills the program).\n");
   }
#else
   {
      unsigned i;
      for (i = 0; i < n_reader; i++) {
         struct stat st;
         if (strcmp(input_paths[i], "-") == 0) {
            LOG(WARN, "One of the input files is STDIN, cannot get file size.\n");
            total_blk = 0;
            break;
         }
         r = stat(input_paths[i], &st);
         verify(r == 0);
         if (st.st_size > used_size) {
            st.st_size = used_size;
         }
         total_blk += (st.st_size + blk_sz - 1) / blk_sz;
      }
   }
#endif

   show_params();
   early_stop = 0;
#if USE_MPI
#else
   signal(SIGTERM, stop);
   signal(SIGINT, status);
#endif

   run();

#if USE_MPI
   r = MPI_Finalize();
   verify(r == MPI_SUCCESS);
   LOG(LOCAL, "shutdown.\n");
#endif

   cout << endl;
   return 0;
}

// some general routines 

static inline void use_it (int r) {
}
static inline float sqr (float x) {
   return x * x;
}

// l2 distance
static inline float l2sqr (const float *p1, const float *p2, unsigned D) {
   float l = 0;
   unsigned d;
   // we hard coded DIM so this loop can be unrolled better by the compiler
   for (d = 0; d < D; d++) {
      l += sqr(p1[d] - p2[d]);
   }
   return l;
}

// quick version of l2 distance
// it's actually l2distance - n2, but n2 is the same for the same query, so not
// adding it should not change the nearest neigbor
static inline float l2sqr_q (const float *p1, float n1, const float *p2, unsigned D) {
   float l = 0;
   unsigned d;
   // we hard coded DIM so this loop can be unrolled better by the compiler
   for (d = 0; d < D; d++) {
      l += p1[d] * p2[d];
   }
   return n1 - 2 * l;
}

// 2-norm
static inline float n2sqr (const float *p1, unsigned D) {
   float l = 0;
   unsigned d;
   // we hard coded DIM so this loop can be unrolled better by the compiler
   for (d = 0; d < D; d++) {
      l += sqr(p1[d]);
   }
   return l;
}

// statistics
typedef struct {
   int cnt;
   float sum;
   float sum2;
   float max, min;
} stat_t;

static inline void stat_reset (stat_t *stat) {
   stat->cnt = stat->sum = stat->sum2 = 0;
   stat->max = -FLT_MAX;
   stat->min = FLT_MAX;
}

static inline void stat_sample (stat_t *stat, float s) {
   stat->sum += s;
   stat->sum2 += sqr(s);
   stat->cnt += 1.0;
   if (s > stat->max) stat->max = s;
   if (s < stat->min) stat->min = s;
}

static inline void stat_merge (stat_t *to,
                               stat_t *from) {
   to->sum += from->sum;
   to->sum2 += from->sum2;
   to->cnt += from->cnt;
   if (from->max > to->max) to->max = from->max;
   if (from->min < to->min) to->min = from->min;
}

static inline float stat_avg (stat_t *stat) {
   return stat->sum / stat->cnt;
}

static inline float stat_std (stat_t *stat) {
   if (stat->cnt > 1) {
      return sqrt( (  stat->sum2
                      - stat_avg(stat) * stat->sum)
                   /(stat->cnt - 1)); 
   }
   else return 0; 
}

static inline float stat_max (stat_t *stat) {
   return stat->max;
}

static inline float stat_min (stat_t *stat) {
   return stat->min;
}

static inline void stat_report (int level, stat_t *stat, const char *name) {
   LOG(level, "%s avg: %g\tstd: %g\tmax: %g\tmin: %g\n",
       name,
       stat_avg(stat),
       stat_std(stat),
       stat_max(stat),
       stat_min(stat));
}

// binary file reading
extern int ftruncate(int fd, off_t length);

// -1 : error
// 0 : read all
// x : file ends, left x bytes to be read
static inline ssize_t readx (int fd, char *p, size_t r, const char *path) {
   ssize_t lr;
   unsigned retry = 0;
   for (;;) {
      lr = read(fd, p, r);
      if (lr > 0) { // something was read
         retry = 0;
         r -= lr;
         p += lr;
         if (r == 0) break; // finished reading the block
      }
      else if (lr == 0) { // file ends
         LOG(LOCAL, "End of %s reached.\n", path);
         break;
      }
      else { //  something funny happen
         if (retry == max_retry) {
            LOG(ERROR, "Error reading %s, give up.\n", path);
            return -1;
         }
         retry++;
         LOG(WARN, "Error reading %s, retry %d...\n", path, retry);
      }
   }
   return r;
}

// -1 : error
// 0 : wrote all
static inline ssize_t writex (int fd, const char *p, size_t r, const char *path) {
   ssize_t lr;
   unsigned retry = 0;
   for (;;) {
      lr = write(fd, p, r);
      if (lr > 0) { // something was written
         retry = 0;
         r -= lr;
         p += lr;
         if (r == 0) break; // finished reading the block
      }
      else { //  something funny happen
         if (retry == max_retry) {
            LOG(ERROR, "Error writing %s, give up.\n", path);
            return -1;
         }
         retry++;
         LOG(WARN, "Error writing %s, retry %d...\n", path, retry);
      }
   }
   return 0;
}

static inline void read_file (const char *path, char *buf, size_t size) {
   int fd, r;
   fd  = open(path, O_RDONLY);
   if (fd == -1) LOG(FATAL, "Cannot open %s.\n", path);
   if (readx(fd, buf, size, path) != 0) verify(0);
   r = close(fd);
   verify(r == 0);
}

static inline void write_file (const char *path, const char *buf, size_t size) {
   int fd, r;
   fd  = open(path, O_WRONLY | O_CREAT, DEFAULT_MODE);
   if (fd == -1) LOG(FATAL, "Cannot open %s.\n", path);
   r = ftruncate(fd, 0);
   verify(r == 0);
   if (writex(fd, buf, size, path) != 0) verify(0);
   r = close(fd);
   verify(r == 0);
}

// 1 if good, 0 if bad, bad values replaced with fill_value
int check_number (float *first, size_t n) {
   char buf[BUFSIZ];
   size_t i, j, b, e;
   int o, s, r;
   int bad = 0;
   for (i = 0; i < n; i++) {
      if (isfinite(first[i])) continue;
      b = i < 2 ? 0 : i - 2;
      e = i + 3;
      if (e > n) e = n;
      o = 0;
      s = BUFSIZ;
      for (j = b; j < e; j++) {
         r = snprintf(buf + o, s, " %g", first[j]);
         if (r >= s) {
            break;
         }
         o += r;
         s -= r;
      }
      LOG(DEBUG, "Bad number: ...%s...\n", buf);

      first[i] = fill_value;
      bad++;
   }
   return bad == 0;
}


// text file reading
#define TXT_OK  0
#define TXT_EOF 1
#define TXT_ERR 2

typedef struct {
   const char *path;
   char buf[TXT_BUF_SIZE_P1];
   int fd;
   char *cur, *end;
   int flag;
} txt_t;

static inline int txt_ok (txt_t *txt) {
   return txt->flag == TXT_OK;
}

static inline int txt_eof (txt_t *txt) {
   return txt->flag == TXT_EOF;
}

static inline int txt_err (txt_t *txt) {
   return txt->flag == TXT_ERR;
}

static inline void txt_open (txt_t *txt, const char *path) {
   txt->path = path;
   if (strcmp(path, "-") == 0) {
      txt->fd = 0;
   }
   else {
      txt->fd = open(path, O_RDONLY);
      if (txt->fd == -1) LOG(FATAL, "Cannot open %s.\n", path);
   }
   txt->cur = txt->end = txt->buf;
   txt->flag = TXT_OK;
   txt->buf[TXT_BUF_SIZE] = 0;
}

static inline void txt_close (txt_t *txt) {
   int r = close(txt->fd);
   verify(r == 0);
}

static inline void txt_read (txt_t *txt, int keep) {
   char *p = txt->buf;
   ssize_t r = 0;
   unsigned i;
   if (keep) {
      char *q = txt->cur;
      while (q < txt->end) {
         *p++ = *q++;
      }
   }
   txt->cur = txt->buf;
   txt->end = p;
   for (i = 0; i < max_retry; i++) {
      r = read(txt->fd, p, TXT_BUF_SIZE - (p - txt->buf));
      if (r >= 0) break;
      LOG(WARN, "Error reading %s, retry %d...\n", txt->path, i + 1);
   }
   if (r == 0) {
      txt->flag = TXT_EOF;
   }
   else if (r < 0) {
      txt->flag = TXT_ERR;
   }
   else {
      txt->end += r;
      verify(txt->end <= &txt->buf[TXT_BUF_SIZE]);
   }
   *txt->end = 0;
}

// seek and return p, so that *p == c or *p == EOL, or p == NULL if EOF is reached
// by no c or EOL has been seen
static inline char *txt_seek_char_or_eol (txt_t *txt, int c, int keep) {
   char *p = txt->cur;
   for (;;) {
      if (p >= txt->end) {
         size_t off = keep ? p - txt->cur : 0;
         txt_read(txt, keep);
         if (!txt_ok(txt)) return NULL;
         p = txt->buf + off;
      }
      if (*p == c || *p == TXT_LINE_DELIM) return p;
      p++;
   }
}

static inline void txt_skip_line (txt_t *txt) {
   char *p = txt_seek_char_or_eol(txt, -1, 0);
   if (p) txt->cur = p + 1;
}

static inline void txt_skip_field (txt_t *txt) {
   char *p = txt_seek_char_or_eol(txt, TXT_FIELD_DELIM, 0);
   if (p && *p != TXT_LINE_DELIM) txt->cur = p + 1;
}

// -1 bad line
// 1 good
// 0 good, with bad values replaced
static inline int txt_read_vec (txt_t *txt, float *v, unsigned dim) {
   unsigned good = 1;
   unsigned i;
   for (i = 0; i < dim; i++) {
      char *bp, *ep;
      char *p;
      p = txt_seek_char_or_eol(txt, TXT_FIELD_DELIM, 1);
      if (txt->cur >= txt->end || *txt->cur == TXT_LINE_DELIM) break;
      bp = txt->cur;
      v[i] = strtod(bp, &ep);
      if (!isfinite(v[i]) || (bp == ep)) {
         v[i] = fill_value;
         good = 0;
      }
      if (p == NULL) { // we need to be careful about eof
         txt->cur = txt->end;
      }
      else if (*p == TXT_LINE_DELIM) {
         txt->cur = p;
      }
      else {
         txt->cur = p + 1;
      }
   }
   return i < dim ? -1 : good;
}


// buffer management

typedef struct {
   unsigned good;
   unsigned partition;
} meta_t;

typedef struct block {
   char *data;
   size_t size;
   meta_t *meta;
   struct block *next;
} block_t;


typedef struct {
   block_t *next;
   block_t *tail;          // when next == NULL, tail is undefined
   pthread_mutex_t lock;
   pthread_cond_t avail;
} queue_t;

static inline void queue_init (queue_t *queue) {
   int r;
   r = pthread_mutex_init(&queue->lock, NULL);
   verify(r == 0);
   r = pthread_cond_init(&queue->avail, NULL);
   verify(r == 0);
   queue->next = queue->tail = NULL;
}

static inline void queue_cleanup (queue_t *queue) {
   int r;
   r = pthread_mutex_destroy(&queue->lock);
   verify(r == 0);
   r = pthread_cond_destroy(&queue->avail);
   verify(r == 0);
}

static inline int queue_empty (queue_t *queue) {
   return queue->next == NULL;
}

static inline void queue_swap (queue_t *a, queue_t *b) {
   block_t *t = a->next;
   a->next = b->next;
   b->next = t;
   t = a->tail;
   a->tail = b->tail;
   b->tail = t;
}

static inline unsigned queue_size (queue_t *queue) {
   block_t *q = queue->next;
   unsigned c = 0;
   while (q != NULL) {
      c++;
      q = q->next;
   }
   return c;
}

static inline void queue_wakeall (queue_t *queue) {
   pthread_cond_broadcast(&queue->avail);
}

void enqueue (queue_t *queue, block_t *block) {
   if (queue->next == NULL) {
      queue->next = queue->tail = block;
   }
   else {
      queue->tail->next = block;
      queue->tail = block;
   }
}
// add cnt to the counter
void enqueue_sync (queue_t *queue, block_t *block) {
   int r;
   block->next = NULL;
   r = pthread_mutex_lock(&queue->lock);
   verify(r == 0);
   enqueue(queue, block);
   r = pthread_mutex_unlock(&queue->lock);
   verify(r == 0);
   r = pthread_cond_signal(&queue->avail);
   verify(r == 0);
}

// if flag != NULL, then check *flag when queue is empty
// if *flag == 0, will return NULL when queue is empty
block_t *dequeue_sync (queue_t *queue, int *flag) {
   block_t *b;
   int r;
   r = pthread_mutex_lock(&queue->lock);
   verify(r == 0);
   while ((queue->next == NULL)
          && (flag == NULL || *flag != 0) && !early_stop) {
      pthread_cond_wait(&queue->avail, &queue->lock);
      verify(r == 0);
   }
   b = queue->next;
   if (b != NULL) {
      queue->next = b->next;
   }
   r = pthread_mutex_unlock(&queue->lock);
   verify(r == 0);
   return b;
}

// atomic counting routines
typedef struct {
   pthread_mutex_t lock;
   unsigned long long cnt;
} atom_cnt_t;

static inline void atom_cnt_init (atom_cnt_t *atm) {
   int r = pthread_mutex_init(&atm->lock, NULL);
   verify(r == 0);
}

static inline void atom_cnt_cleanup (atom_cnt_t *atm) {
   int r = pthread_mutex_destroy(&atm->lock);
   verify(r == 0);
}

static inline void atom_cnt_add (atom_cnt_t *atm, unsigned long long val) {
   int r;
   r = pthread_mutex_lock(&atm->lock);
   verify(r == 0);
   atm->cnt += val;
   r = pthread_mutex_unlock(&atm->lock);
   verify(r == 0);
}

// timer routines
typedef time_t mytimer_t;

static inline void timer_reset (mytimer_t *t) {
   *t = time(NULL);
}

static inline float timer_elapsed (mytimer_t *t) {
   return time(NULL) - *t;
}

// searching routines
void kd_init ();
void kd_index ();
unsigned kd_search (const float *pt, unsigned *cnt);
void kd_cleanup ();
unsigned ln_search (const float *pt);

// now the real stuff ...

// global data structure
// use static data so the whole KD tree is static
float *means;           // ith vector: means + i * dim
#if USE_BLAS
float *means_n2sqr;
float *dots;
#endif

float *backups;         // ith vector: backups + i * dim
// working area of the threads
double *ksums;
unsigned *kcnts;
block_t *blocks;

queue_t init_q, wait_q, partition_q, done_q;
atom_cnt_t  reader_cnt, worker_cnt, rd_blk_cnt, done_blk_cnt, done_vec_cnt;
int all_fit_in;
mytimer_t timer_all;
mytimer_t timer_loop;

void status (int signum) {
   LOG(SIGNAL, "SIGINT (Ctrl+C) received.\n");
   LOG(SIGNAL, "Current progress:\n");
   LOG(SIGNAL, "\treading threads = %lu,\n", n_reader);
   LOG(SIGNAL, "\tworking threads = %lu,\n", n_worker);
   LOG(SIGNAL, "\tblocks size = %zu,\n", blk_sz);
   LOG(SIGNAL, "\t%llu blocks read,\n", rd_blk_cnt.cnt);
   LOG(SIGNAL, "\t%llu blocks processed,\n", done_blk_cnt.cnt);
   LOG(SIGNAL, "\t%llu vectors clustered.\n", done_vec_cnt.cnt);
   LOG(SIGNAL, "%g seconds have passed since this loop started.\n",
       timer_elapsed(&timer_loop));
   LOG(SIGNAL, "%g seconds have passed since the program started.\n",
       timer_elapsed(&timer_all));
   if (total_blk > 0) {
      LOG(SIGNAL, "%2.1f%% current loop done.\n",
          100.0 * done_blk_cnt.cnt / total_blk);
   }
   LOG(SIGNAL, "Use \"killall -s KILL kmeans\" to kill the process.\n");
   signal(SIGINT, status);
}

void stop (int signum) {
   early_stop = 1;
   queue_wakeall(&init_q);
   queue_wakeall(&wait_q);
   queue_wakeall(&done_q);
   LOG(SIGNAL, "SIGTERM received, informed readers and workers to stop.\n");
}

void *txt_reader (void *param);
void *reader (void *param);
void *worker (void *param);
void *partitioner (void *param);
void seed (void);
int update_means (void);

void write_size (int n) {
   FILE *fout;
   char buf[BUFSIZ];
   unsigned i;
   int r;
   if (strlen(size_path) == 0) return;
   if (n >= 0) {
      r = snprintf(buf, BUFSIZ, "%s.%u", size_path, n);
      verify(r < BUFSIZ);
   }
   else {
      strncpy(buf, size_path, BUFSIZ);
   }
   LOG(INFO, "Saving cluster sizes at %s.\n", buf);

   fout = fopen(buf, "w");
   if (fout == NULL) LOG(FATAL, "Cannot open file %s.\n", buf);

   for (i = 0; i < K; i++) {
      fprintf(fout, "%u\n", kcnts[i]);
   }

   fclose(fout);
}

void run (void) {

   unsigned i, k; 
   int r;
   void *null;
   pthread_t *ths;
   int converged = 0;

   timer_reset(&timer_all);
   // alloc dynamic data

   means = (float *)malloc(K * vec_size);
   verify(means);

#if USE_BLAS
   means_n2sqr = (float *)malloc(K * sizeof(float));
   verify(means_n2sqr);
   cout << "n_worker = " << n_worker << " K = " << K << endl;
   cout << "blk_vec = " << blk_vec << endl;
   cout << "n_worker*K*blk_vec*sizeof(float) = " 
        << 1E-9*n_worker*K*blk_vec*sizeof(float) << " Gbytes" << endl;
   dots = (float *)malloc(n_worker * K * blk_vec * sizeof(float));
   verify(dots);
#endif

   backups = 0;
   if (refill) {
      if (is_root) { // n_backup == n_backup_node for non-MPI mode
         backups = (float *)malloc(n_backup * vec_size);
      }
      else {
         backups = (float *)malloc(n_backup_node * vec_size);
      }
      verify(backups);
   }

   ksums = (double *)malloc(n_worker * Kdim * sizeof(double));
   verify(ksums);
   kcnts = (unsigned *)malloc(n_worker * K * sizeof(unsigned));
   verify(kcnts);

   queue_init(&init_q);
   queue_init(&wait_q);
   queue_init(&partition_q);
   queue_init(&done_q);

   blocks = (block_t *)malloc(n_buf_blk * sizeof(block_t));
   verify(blocks);

   for (i = 0; i < n_buf_blk; i++) {
      blocks[i].data = (char *)malloc(blk_sz);
      blocks[i].meta = (meta_t *)malloc(blk_vec * sizeof(meta_t));
      verify(blocks[i].data);
      enqueue(&init_q, &blocks[i]);
   }

   atom_cnt_init(&reader_cnt);
   atom_cnt_init(&worker_cnt);
   atom_cnt_init(&rd_blk_cnt);
   atom_cnt_init(&done_blk_cnt);
   atom_cnt_init(&done_vec_cnt);

#if USE_KD_TREE
   kd_init();
#endif

   ths = (pthread_t *)malloc(n_th * sizeof(pthread_t));
   verify(ths);

   all_fit_in = 0;

   for (k = 0; k < max_it && !converged; k++) { 

      timer_reset(&timer_loop);

      if (is_root) {
         LOG(INFO, "Loop %u starts.\n", k);
      }

      rd_blk_cnt.cnt = 0;
      done_blk_cnt.cnt = 0;
      done_vec_cnt.cnt = 0;

      if (!all_fit_in) {
         reader_cnt.cnt = n_reader;
         for (i = 0; i < n_reader; i++) {
            r = pthread_create(&ths[i], NULL,
                               txt_input ? txt_reader : reader,
                               (void *)(size_t)i);
            verify(r == 0);
         }

         if (k == 0) {
            seed();
         }
      }

#if USE_KD_TREE
      kd_index();
#endif

#if USE_BLAS
      for (i = 0; i < K; i++) {
         means_n2sqr[i] = n2sqr(means + i * dim, dim);
      }
#endif
      worker_cnt.cnt = n_worker;
      for (i = 0; i < n_worker; i++) {
         r = pthread_create(&ths[i + n_reader], NULL,
                            worker, (void *)(size_t)i);
         verify(r == 0);
      }

      if (n_partitioner > 0) {
         verify(n_partitioner == 1);
         r = pthread_create(&ths[n_reader + n_worker], NULL, partitioner, (void *)(size_t)k);
         verify(r == 0);
      }

      for (i = 0; i < n_th; i++) {
         if (all_fit_in && i < n_reader) continue;
         r = pthread_join(ths[i], &null);
         verify(r == 0);
      }

      if (early_stop) {
         if (is_root) {
            LOG(INFO, "Loop broken.\n");
         }
         break;
      }

      if (!all_fit_in) {
         if (rd_blk_cnt.cnt <= n_buf_blk) {
            all_fit_in = 1;
            if (is_root) {
               LOG(INFO, "All data are in memory.\n");
            }
            verify(rd_blk_cnt.cnt == queue_size(&done_q));
         }
      }

      verify(queue_size(&wait_q) == 0);
      verify(queue_size(&init_q) + queue_size(&done_q) == n_buf_blk);

      if (all_fit_in) {
         queue_swap(&wait_q, &done_q);
      }
        
      if (n_partitioner) {
         converged = 1;       // for partitioning, one loop is enough
      }
      else  {
         converged = update_means();
         // write out the result every loop
         if (is_root && save_loop) {
            char buf[BUFSIZ];
            snprintf(buf, BUFSIZ, "%s.%u", output_path, k);
            LOG(INFO, "Saving results at %s.\n", buf);
            write_file(buf, (const char *)means, Kdim * sizeof(float));
            write_size(k);
         }
      }

#if USE_MPI
      r = MPI_Reduce(is_root ? MPI_IN_PLACE : &rd_blk_cnt.cnt,
                     &rd_blk_cnt.cnt, 1, MPI_UNSIGNED_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
      verify(r == MPI_SUCCESS);
      r = MPI_Reduce(is_root ? MPI_IN_PLACE : &done_blk_cnt.cnt,
                     &done_blk_cnt.cnt, 1, MPI_UNSIGNED_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
      verify(r == MPI_SUCCESS);
      r = MPI_Reduce(is_root ? MPI_IN_PLACE : &done_vec_cnt.cnt,
                     &done_vec_cnt.cnt, 1, MPI_UNSIGNED_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
      verify(r == MPI_SUCCESS);

#endif
      if (is_root) {
         LOG(INFO, "%u blocks read.\n", rd_blk_cnt.cnt);
         LOG(INFO, "%u blocks processed.\n", done_blk_cnt.cnt);
         LOG(INFO, "%u vectors clustered.\n", done_vec_cnt.cnt);
         LOG(INFO, "Loop %u finished, using %g seconds.\n",
             k, timer_elapsed(&timer_loop));
      }
   }

   if (is_root && !n_partitioner) {

      if (k >= max_it) {
         LOG(INFO, "Maximal iteration number reached.\n");
      }
      else {
         LOG(INFO, "Clustering stopped in %u loops.\n", k + 1);
      }

      LOG(INFO, "Saving results.\n");
      write_file(output_path, (const char *)means, Kdim * sizeof(float));
      write_size(-1);
   }

   // cleanup data

#if USE_KD_TREE
   kd_cleanup();
#endif

   free(ths);

   atom_cnt_cleanup(&reader_cnt);
   atom_cnt_cleanup(&worker_cnt);
   atom_cnt_cleanup(&rd_blk_cnt);
   atom_cnt_cleanup(&done_blk_cnt);
   atom_cnt_cleanup(&done_vec_cnt);

   queue_cleanup(&init_q);
   queue_cleanup(&wait_q);
   queue_cleanup(&partition_q);
   queue_cleanup(&done_q);

   for (i = 0; i < n_buf_blk; i++) {
      free(blocks[i].data);
      free(blocks[i].meta);
   }
   free(blocks);

   free(kcnts);
   free(ksums);

   if (refill) {
      free(backups);
   }

   free(means);
#if USE_BLAS
   free(means_n2sqr);
   free(dots);
#endif

   if (is_root) {
      LOG(INFO, "%g seconds used in all.\n", timer_elapsed(&timer_all));
   }
}

void *txt_reader (void *param) {

   size_t id = (size_t)param;
   const char *path;
   txt_t txt;
   unsigned cnt = 0, lcnt_total = 0, ecnt_total = 0;
   unsigned i;

   mytimer_t timer;

   timer_reset(&timer);

   path = input_paths[id];
   LOG(LOCAL, "Reader %zu: Running, txt input is %s.\n", id, path);
   if (strcmp(path, "-") == 0) {
#if USE_MPI
      LOG(FATAL, "Reader %zu: STDIN not supported in MPI mode.\n", id);
#else
      LOG(LOCAL, "Reader %zu: Using STDIN as input.\n", id);
#endif
   }

   txt_open(&txt, path);

   // read head_size lines
   for (i = 0; i < head_size; i++) {
      txt_skip_line(&txt);
      if (!txt_ok(&txt)) break;
   }

   while (txt_ok(&txt)) {
      unsigned lcnt, ecnt;
      char *next; // next vector

      // get a block
      block_t *blk = NULL;

      if (early_stop) {
         LOG(LOCAL, "Reader %11u: Early stop.\n", id);
         break;
      }

      if (!queue_empty(&init_q)) {
         blk = dequeue_sync(&init_q, &all_fit_in);
      }

      if (blk == NULL) {
         blk = dequeue_sync(&done_q, NULL);
         if (blk == NULL) {
            // this should not happen
            LOG(ERROR, "Reader %zu: Dequeue failure caused by SIGTERM or bugs.\n", id);
            break;
         }
      }
    
      // read to fill the block
      next = blk->data;
      lcnt = ecnt = 0;

      while (lcnt < blk_vec) {

         int r;

         // read a vector
         blk->meta[lcnt].good = 1;

         // skip c columns
         for (i = 0; i < skip_field; i++) {
            txt_skip_field(&txt);
            if (!txt_ok(&txt)) break;
         }
         if (!txt_ok(&txt)) break;

         r = txt_read_vec(&txt, (float *)next, dim);

         if (r == -1) {
            LOG(WARN, "Reader %zu: bad line.\n", id);
            if (!txt_ok(&txt)) break;
            ecnt++;
         }
         else {
            blk->meta[lcnt].good = r;
            next += stride;
            lcnt++;
         }
         txt_skip_line(&txt);
      }

      blk->size = next - blk->data;

      LOG(LOCAL, "Reader %zu: %u lines read, %u bad lines.\n", id, lcnt, ecnt);
      cnt++;
      lcnt_total += lcnt;
      ecnt_total += ecnt;

      enqueue_sync(&wait_q, blk);

      atom_cnt_add(&rd_blk_cnt, 1);
   }

   txt_close(&txt);
   atom_cnt_add(&reader_cnt, -1);
   LOG(LOCAL, "Reader %zu done, %u blocks, %u lines read, %u bad lines, %g seconds.\n",
       id, cnt, lcnt_total, ecnt_total, timer_elapsed(&timer));
   queue_wakeall(&wait_q);
   return NULL;
}

void prep (block_t *blk) {
   char *next = blk->data;
   char *end = blk->data + blk->size + 1 - vec_size;
   unsigned i = 0;

   while (next < end) {
      float *pt = (float *)next;
      blk->meta[i].good = check_number(pt, dim);
      next += stride;
      i++;
   }
}

void *reader (void *param) {

   size_t id = (size_t)param;
   const char *path;
   int fd = -1;
   ssize_t lr;
   int r;
   unsigned cnt = 0;
   size_t off, end, to_read;
   mytimer_t timer;

   timer_reset(&timer);

   path = input_paths[id];
   LOG(LOCAL, "Reader %zu: Running, input is %s.\n", id, path);
   if (strcmp(path, "-") == 0) {
#if USE_MPI
      LOG(FATAL, "Reader %zu: STDIN not supported in MPI mode.\n", id);
#else
      LOG(LOCAL, "Reader %zu: Using STDIN as input.\n", id);
#endif
      fd = 0;
   }
   else {
      verify(!all_fit_in);
      fd  = open(path, O_RDONLY);
      if (fd == -1) LOG(FATAL, "Reader %zu: Cannot open file.\n", id);
   }

   off = head_size;
   end = used_size;
#if USE_MPI
   if (!local_file) {
      // shrink the size of the region we are 
      struct stat st;
      unsigned total, blk_node;
      r = fstat(fd, &st);
      verify(r == 0);
      if (st.st_size > used_size) {
         st.st_size = used_size;
      }
      total = (st.st_size - head_size + blk_sz - 1) / blk_sz;
      blk_node = (total + mpi_size - 1) / mpi_size;
      off = head_size + blk_node * mpi_rank * blk_sz;
      end = off + blk_node * blk_sz;
      if (end > st.st_size) {
         end = st.st_size;
      }
      LOG(LOCAL, "Reader %zu: using [%zu, %zu) of %s.\n", id, off, end, path);
   }
#endif

   lr = lseek(fd, off, SEEK_SET);
   verify(lr == off);

   for (;;) {
      block_t *blk = NULL;

      if (early_stop) {
         LOG(LOCAL, "Reader %11u: Early stop.\n", id);
         break;
      }

      if (!queue_empty(&init_q)) {
         blk = dequeue_sync(&init_q, &all_fit_in);
      }

      if (blk == NULL) {
         blk = dequeue_sync(&done_q, NULL);
         if (blk == NULL) {
            // this should not happen
            LOG(ERROR, "Reader %zu: Dequeue failure caused by SIGTERM or bugs.\n", id);
            break;
         }
      }

      to_read = blk_sz;
      if (off + blk_sz > end) {
         to_read = end - off;
      }

      lr = readx(fd, blk->data, to_read, path);
      if (lr == -1) {
         LOG(ERROR, "Reader %zu: File read failed.\n", id);
         break;
      }
      blk->size = to_read - lr;

      off += blk->size;

      LOG(LOCAL, "Reader %zu: %zu bytes read.\n", id, blk->size);
      cnt++;

      prep(blk);

      enqueue_sync(&wait_q, blk);

      atom_cnt_add(&rd_blk_cnt, 1);

      if (lr > 0) {
         LOG(LOCAL, "Reader %zu: last available byte read.\n", id);
         break;    // EOF
      }
      if (off >= end) {
         LOG(LOCAL, "Reader %zu: reaching user set size.\n", id);
         break;
      }
   }
   r = close(fd);
   verify(r == 0);
   atom_cnt_add(&reader_cnt, -1);
   LOG(LOCAL, "Reader %zu done, %u blocks read in %g seconds.\n",
       id, cnt, timer_elapsed(&timer));
   queue_wakeall(&wait_q);
   return NULL;
}

void *worker (void *param) {
   size_t id = (size_t)param;
   unsigned long long vec_cnt = 0;
   unsigned blk_cnt = 0;
   unsigned long long chance;
   double *ksum = ksums + id * Kdim;
   unsigned *kcnt = kcnts + id * K;
#if USE_BLAS
   float *dot = dots + id * K * blk_vec;
#endif

   mytimer_t all_timer;
   mytimer_t blk_timer;

#if USE_KD_TREE
   stat_t kd_stat;
#endif

   unsigned bk_len = n_backup_node / n_worker;
   unsigned bk_off = id * bk_len;
   // we will sample the bk_len backup vectors starting from bk_off
   //

   verify(id < n_worker);
   LOG(LOCAL, "Worker %zu: Running.\n", id);

   timer_reset(&all_timer);

   memset(ksum, 0, Kdim * sizeof(double));
   memset(kcnt, 0, K * sizeof(float));

   for (;;) {
      const char *next, *end;
      unsigned vc = 0;
      unsigned idx = 0;
      block_t *blk = dequeue_sync(&wait_q, (int *)&reader_cnt.cnt);
      if (blk == NULL) break;

      timer_reset(&blk_timer);

#if USE_KD_TREE
      stat_reset(&kd_stat);
#endif

      next = blk->data;
      end = blk->data + blk->size + 1 - vec_size;
#if USE_BLAS
      memset(dot, 0, K * blk_vec * sizeof(float));
      cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasTrans, blk_vec, K, dim, -2.0, (const float *)next, cblas_lda, means, dim, 0, dot, K);
#endif

      while (next < end) {
         const float *pt = (const float *)next;
         unsigned c, d;
#if USE_KD_TREE
         unsigned kd_cnt;
#endif

         next += stride;
         if (early_stop) {
            LOG(LOCAL, "Worker %zu: Early stop.\n", id);
            goto end;
         }
#if CHECK_NUMBER
         if (!blk->meta[idx].good && skip_bad_number) 
         {
            idx++;
            continue;
         }
#endif
         // OK, here's the k-means part...
#if USE_KD_TREE
         c = kd_search(pt, &kd_cnt);
         stat_sample(&kd_stat, kd_cnt);
#if VERIFY_KD_TREE
         {
            unsigned cc = ln_search(pt);
            if (c != cc) {
               float l1 = l2sqr(pt, means + c * dim, dim);
               float l2 = l2sqr(pt, means + cc * dim, dim);
               LOG(WARN, "KD TREE: %u, %f, LN SCAN: %u, %f\n", c, l1, cc, l2);
            }
         }
#endif
#elif USE_BLAS
         {
            float *d_off = dot + idx * K;
            float min = means_n2sqr[0] + d_off[0];
            unsigned cc;
            c = 0;
            for (cc = 1; cc < K; cc++) {
               float d = means_n2sqr[cc] + d_off[cc];
               if (d < min) {
                  min = d;
                  c = cc;
               }
            }
         }
#else
         c = ln_search(pt);
#endif
         blk->meta[idx].partition = c;

         kcnt[c]++;
         {
            double *cs = ksum + c * dim;
            float *cm = means + c * dim;
            for (d = 0; d < dim; d++) {
               cs[d] += pt[d] - cm[d];
            }
         }
         vc++;

         // sample backup
         if (refill) {
            chance = vec_cnt / bk_len + 10;
            if (rand() % chance == 0) {
               memcpy(backups + 
                      (bk_off + rand() % bk_len) * dim,
                      pt, vec_size);
            }
         }

         idx++;
      }
      if (n_partitioner) {
         enqueue_sync(&partition_q, blk);
      }
      else {
         enqueue_sync(&done_q, blk);
      }

#if USE_KD_TREE
      stat_report(DEBUG, &kd_stat, "KD-tree");
#endif
      LOG(LOCAL, "Worker %zu: %u vectors processed in %g seconds.\n",
          id, vc, timer_elapsed(&blk_timer));
      atom_cnt_add(&done_blk_cnt, 1);
      atom_cnt_add(&done_vec_cnt, vc);
      vec_cnt += vc;
      blk_cnt++;
   }
   LOG(LOCAL, "Worker %zu done, %llu vectors in %u blocks processed in %g seconds.\n",
       id, vec_cnt, blk_cnt, timer_elapsed(&all_timer));
  end:
   atom_cnt_add(&worker_cnt, -1);
   return NULL;
}

void* partitioner (void*dummy) {

   char path[BUFSIZ];
   int *fid;
   unsigned i;
   int r;
   unsigned count = 0;

   fid = (int *)malloc(K * sizeof(int));
   verify(fid);

   for (i = 0; i < K; i++) {
      r = snprintf(path, BUFSIZ, "%s-%u", output_path, i);
      verify(r < BUFSIZ);
      fid[i] = open(path, O_WRONLY | O_CREAT | O_TRUNC, DEFAULT_MODE);
      if (fid[i] == -1) LOG(FATAL, "Cannot open %s.\n", path);
   }

   for (;;) {
      const char *next, *end;
      unsigned idx = 0;

      block_t *blk = dequeue_sync(&partition_q, (int *)&worker_cnt.cnt);
      if (blk == NULL) break;

      if (early_stop) break;

      next = blk->data;
      end = blk->data + blk->size + 1 - vec_size;

      while (next < end) {
         const char *pt = next;
         unsigned p;

         next += stride;

#if CHECK_NUMBER
         if (!blk->meta[idx].good && skip_bad_number) continue;
#endif
         p = blk->meta[idx].partition;
         verify(p < K);
         writex(fid[p], pt, vec_size, output_path);
         count++;
         idx++;
      }
      enqueue_sync(&done_q, blk);
   }
   for (i = 0; i < K; i++) {
      r = close(fid[i]);
      verify(r == 0);
   }
   free(fid);
   LOG(LOCAL, "%u vectors partitioned.\n", count);
   return NULL;
}

void seed (void) {
   int r = 0;

   use_it(r);
   // read seed if provided
   if (seeded && (is_root || !local_file)) {
      read_file(seed_path, (char *)means, Kdim * sizeof(float));
      if (!check_number(means, Kdim)) {
         LOG(ERROR, "You have bad numbers in the seed file.\n");
      }
   }

   if (n_seed_vec > 0) {
      unsigned vpb = blk_vec;
      unsigned pool_size = vpb * n_hld_blk;
      block_t **pb;
      unsigned *idx;
      unsigned i, j, t;
      pb = (block_t **)malloc(n_hld_blk * sizeof(block_t *));
      verify(pb);
      idx = (unsigned *)malloc(pool_size * sizeof(unsigned));
      verify(idx);
      verify(pool_size >= n_seed_vec);
      LOG(INFO, "Gathering data to produce seed clusters...\n");
      for (i = 0; i < pool_size; i++) {
         idx[i] = i;
      }
      // shuffle index
      for (i = 0; i < pool_size; i++) {
         j = rand() % pool_size;
         t = idx[i], idx[i] = idx[j], idx[j] = t; 
      }
      for (i = 0; i < n_hld_blk; i++) {
         pb[i] = dequeue_sync(&wait_q, NULL);
         if (pb[i] == NULL || pb[i]->size < blk_sz) {
            cout << "i = " << i << " pb[i]->size = " << pb[i]->size
                 << endl;
            cout << "blk_sz = " << blk_sz << endl;
            LOG(FATAL, "Cannot get enough data to produce the seed.\n");
         }
      }

      i = 0;
      if (!seeded) {
         // select the initial cluster centers
         if (is_root) {
            for (j = 0; j < K; i++, j++) {
               float *pt = (float *)(pb[idx[i]/vpb]->data + (idx[i] % vpb) * stride);
               check_number(pt, dim);
               memcpy(means + j * dim, pt, vec_size);
            }
         }
      }

      for (j = 0; j < n_backup_node; i++, j++) {
         memcpy(backups + j * dim, pb[idx[i]/vpb]->data + (idx[i] % vpb) * stride, vec_size);
      }

      verify(n_seed_vec == i);

      for (i = 0; i < n_hld_blk; i++) {
         enqueue_sync(&wait_q, pb[i]);
      }
      free(idx);
      free(pb);
   }

#if USE_MPI
   if (!seeded || local_file) {
      LOG(DEBUG, "Broad casting cluster centers...\n");
      r = MPI_Bcast(means, Kdim, MPI_FLOAT, 0, MPI_COMM_WORLD);
      verify(r == MPI_SUCCESS);
      LOG(DEBUG, "Cluster centers ready.\n");
   }
#endif
   if (is_root) {
      LOG(INFO, "Seed clusters generated.\n");
   }
}

int update_means (void) {

   unsigned k, i, d;
   int r = 0;
   mytimer_t timer;
   int converged;

   timer_reset(&timer);

   use_it(r); // just to avoid warning
   if (is_root) {
      LOG(INFO, "Start updating means...\n");
   }
#if USE_MPI
   // gather backup samples
   if (refill) {
      r = MPI_Gather(is_root ? MPI_IN_PLACE : backups,
                     n_backup_node * dim, MPI_FLOAT,
                     backups, n_backup_node * dim, MPI_FLOAT, 0, MPI_COMM_WORLD);
      verify(r == MPI_SUCCESS);
   }
#endif

   // this is everybody's work
   {
      unsigned *c, *sc = kcnts;
      double *s, *ss = ksums;
      for (k = 0; k < K; k++, sc++, ss += dim) {
         c = sc + K;
         s = ss + Kdim;
         for (i = 1; i < n_worker; i++, c += K, s += Kdim) {
            *sc += *c;
            for (d = 0; d < dim; d++) {
               ss[d] += s[d];
            }
         }
      }
   }

#if USE_MPI
   r = MPI_Reduce(is_root ? MPI_IN_PLACE : ksums,
                  ksums, Kdim, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
   verify(r == MPI_SUCCESS);
   r = MPI_Reduce(is_root ? MPI_IN_PLACE : kcnts,
                  kcnts, K, MPI_UNSIGNED, MPI_SUM, 0, MPI_COMM_WORLD);
   verify(r == MPI_SUCCESS);

   if (mpi_rank == 0) {
#endif
      float *bk_next = backups;
      float *bk_end = backups + n_backup * dim;
      float *cm;
      double *cs;
      unsigned st = 0;

      stat_t size_stat;
      stat_t shift_stat;

      stat_reset(&size_stat);
      stat_reset(&shift_stat);

      cm = means;
      cs = ksums;
      for (k = 0; k < K; k++, cm += dim, cs += dim) {
         float shift, v;
         unsigned cnt = kcnts[k];

         if (cnt == 0) {
            if (bk_next >= bk_end) {
               // we've used up backup vectors,
               // just pretend the cluster is not empty
               // we'll have a cluster center <0, 0, ... , 0>
               cnt = 1;
               st++;
            }
         }

         if (cnt == 0) {
            // now we are sure bk_next < n_backup
            shift = 0;
            for (d = 0; d < dim; d++) {
               shift += sqr(cm[d] - bk_next[d]);
               cm[d] = bk_next[d];
            }
            shift = sqrt(shift);
            bk_next += dim;
         }
         else {
            shift = 0;
            for (d = 0; d < dim; d++) {
               v = cs[d] / cnt;
               shift += sqr(v);
               cm[d] += v;
            }
            shift = sqrt(shift);
         }

         stat_sample(&size_stat, cnt);
         stat_sample(&shift_stat, shift);
      }
      LOG(INFO, "%u empty clusters appear, populated with backup vectors.\n", st);
      if (st > 0) {
         LOG(ERROR, "We are %u backup vectors short.\n", st);
      }

      converged = stat_max(&shift_stat) < delta;

      stat_report(INFO, &size_stat, "cluster size");
      stat_report(INFO, &shift_stat, "center shift");

#if USE_MPI
   }
   r = MPI_Bcast(means, Kdim, MPI_FLOAT, 0, MPI_COMM_WORLD);
   verify(r == MPI_SUCCESS);
   r = MPI_Bcast(&converged, 1, MPI_INT, 0, MPI_COMM_WORLD);
   verify(r == MPI_SUCCESS);

#endif 
   if (is_root) {
      LOG(INFO, "Finish updating means, %g seconds used.\n", timer_elapsed(&timer));
   }

   return converged;
}

unsigned ln_search (const float *pt) {
   unsigned k, c = 0;
   float *cm = means;
   float d;
#if 0
   float min = l2sqr_q(cm, means_n2sqr[0], pt, dim);
#else
   float min = l2sqr(cm, pt, dim);
#endif
   cm += dim;
   for (k = 1; k < K; k++, cm += dim) {
#if 0
      d = l2sqr_q(cm, means_n2sqr[k], pt, dim);
#else
      d = l2sqr(cm, pt, dim);
#endif
      if (d < min) {
         c = k;
         min = d;
      }
   }
   return c;
}

#if USE_KD_TREE

// very compact simplementation of KD-tree
// mainly based on D. Mount's ANN library.
// the data structure is statically allocated
// and dynamically constructed.

typedef struct kd_node kd_node_t;
// node really means internal nodes

union kd_leaf_or_node {
   unsigned long long leaf;    // leaf, a single index into the means array
   const kd_node_t *node;
};
// because all the nodes are statically allocated, the range of
// the address is known, and the range of leaf is from 0 to K-1.
// usually the first node address should be larger than K,
// so we can tell whether it's a leaf or a node simply by looking at the values.
// we check in the 

static inline int kd_is_leaf (union kd_leaf_or_node u) {
   // we have to test with node because node is 64-bit
   // and leaf is 32-bit
   // the lower 32-bit of any 64-bit value can be smaller than K
   //return u.node < (const struct kd_node *)(unsigned long long)K;
   return u.leaf < K;
}

// KD-tree internal node
struct kd_node {                
   unsigned cut_dim;
   float cut_val;
   float lower, upper;         // range of the dimension of this space partition
   union kd_leaf_or_node left, right;
};

kd_node_t *kd_nodes;        // statically allocate all the nodes
unsigned kd_next_node;          // keep the number of node used
#define kd_root kd_nodes     // root is always the first node

static inline kd_node_t *kd_alloc_node (void) {
   return &kd_nodes[kd_next_node++];
}

// lower and upper bound of each dimension  with all the cluster centers
typedef struct {
   float *lo;
   float *hi;
} bnds_t;

bnds_t kd_bnds;

void kd_init (void) {
   kd_nodes = (kd_node_t*)malloc((K-1) * sizeof(kd_node_t));
   verify(kd_nodes);
   kd_bnds.lo = (float *)malloc(dim * sizeof(float));
   verify(kd_bnds.lo);
   kd_bnds.hi = (float *)malloc(dim * sizeof(float));
   verify(kd_bnds.hi);
}

void kd_cleanup (void) {
   free(kd_nodes);
   free(kd_bnds.lo);
   free(kd_bnds.hi);
}


#define ERR 0.001
static inline void bisec (
   unsigned *idx, unsigned n,          // item ids to be divided
   // will be shuffled
   const bnds_t *bnds,                 // bounds of the box to be divided
   unsigned *cut_dim, float *cut_val,
   unsigned *n_lo                      // # items on the lower side
   ) {

   unsigned cd;
   unsigned d, i, t;
   unsigned br1, br2;
   int l, r;
   float cv, ideal_cv;
   float max_len, len;
   float max_spr, spr;
   float cd_min, cd_max;
   cd_min = cd_max = 0;

   // find the longest side of the bounding box
   max_len = bnds->hi[0] - bnds->lo[0];
   for (d = 1; d < dim; d++) {
      len = bnds->hi[d] - bnds->lo[d];
      if (len > max_len) max_len = len;
   }

   // find the dimension with maximal spread as the cutting dimension
   max_spr = -1;
   cd = 0; // just to avoid warning
   for (d = 0; d < dim; d++) {
      len = bnds->hi[d] - bnds->lo[d];

      // if this side is among the longest
      if (len > (1-ERR) * max_len) {

         // take the spread
         float min, max;
         min = max = (means + dim * idx[0])[d];
         for (i = 1; i < n; ++i) {
            float v = (means + dim * idx[i])[d];
            if (v < min) min = v;
            if (v > max) max = v;
         }
         spr = max - min;
         if (spr > max_spr) {
            max_spr = spr;
            cd_min = min;
            cd_max = max;
            cd = d;
         }
      }
   }

   ideal_cv = cv = (bnds->lo[cd] + bnds->hi[cd]) / 2;
    
   if (cv < cd_min) cv = cd_min;
   else if (cv > cd_max) cv = cd_max;

   l = 0;
   r = n - 1;
   for (;;) {
      while (l < n && (means + dim * idx[l])[cd] < cv) l++;
      while (r >= 0 && (means + dim * idx[r])[cd] >= cv) r--;
      if (l > r) break;
      t = idx[l], idx[l] = idx[r], idx[r] = t;
      l++;
      r--;
   }
   br1 = l;            // means[0..br1-1][d] < cv <= means[br1..n-1][d]
   r = n - 1;
   for (;;) {
      while (l < n && (means + dim * idx[l])[cd] <= cv) l++;
      while (r >= 0 && (means + dim * idx[r])[cd] > cv) r--;
      if (l > r) break;
      t = idx[l], idx[l] = idx[r], idx[r] = t;
      l++;
      r--;
   }
   br2 = l;            // means[br1..br2-1][d] = cv < means[br2..n-1][d]

   *cut_dim = cd;
   *cut_val = cv;

   if (ideal_cv < cd_min) *n_lo = 1;
   else if (ideal_cv > cd_max) *n_lo = n - 1;
   else if (br1 > n/2) *n_lo = br1;
   else if (br2 < n/2) *n_lo = br2;
   else *n_lo = n/2;
}

const kd_node_t *kd_index_hlp (unsigned *idx, unsigned n,
                               bnds_t *bnds       // bounds of the subtree
   ) {

   kd_node_t *node;
   unsigned n_lo, n_hi, cut_dim;
   float cut_val;
   float lo, hi;               // save value for the cutted dimension
   assert(n > 1);
   node = kd_alloc_node();

   bisec(idx, n, bnds, &cut_dim, &cut_val, &n_lo);

   node->cut_dim = cut_dim;
   node->cut_val = cut_val;

   lo = node->lower = bnds->lo[cut_dim];
   hi = node->upper = bnds->hi[cut_dim];

   // construct the left subtree
   bnds->hi[cut_dim] = cut_val;
   assert(n_lo > 0);
   if (n_lo == 1) {
      node->left.leaf = idx[0];
   }
   else {
      node->left.node = kd_index_hlp(idx, n_lo, bnds);
   }
   bnds->hi[cut_dim] = hi;

   // construct the right subtree
   bnds->lo[cut_dim] = cut_val;
   n_hi = n - n_lo;
   if (n_hi == 1) {
      node->right.leaf = idx[n-1];
   }
   else {
      node->right.node = kd_index_hlp(idx + n_lo, n_hi, bnds);
   }
   bnds->lo[cut_dim] = lo;

   return node;
}

void kd_index () {
   unsigned *kd_idx;
   unsigned d, i;
   mytimer_t timer;

   LOG(INFO, "Constructing KD-tree...\n");
   timer_reset(&timer);

   kd_idx = (unsigned *)malloc(K * sizeof(unsigned));
   verify(kd_idx);
   kd_next_node = 0;
   verify(kd_root > (kd_node_t *)(size_t)K);

   // init idx to an arbitrary permutation
   for (i = 0; i < K; i++) {
      kd_idx[i] = i;
   }

   // bounding box
   for (d = 0; d < dim; d++) {
      float *cm = means + d;
      kd_bnds.lo[d] = kd_bnds.hi[d] = *cm;
      cm += dim;
      for (i = 1; i < K; i++, cm += dim) {
         if (*cm < kd_bnds.lo[d]) {
            kd_bnds.lo[d] = *cm;
         }
         else if (*cm > kd_bnds.hi[d]) {
            kd_bnds.hi[d] = *cm;
         }
      }
   }

   // recursively construct the tree
   // the first node automatically becomes the root
   kd_index_hlp(kd_idx, K, &kd_bnds);

   verify(kd_next_node == K - 1);

   free(kd_idx);
   LOG(INFO, "KD-tree constructed in %g seconds.\n", timer_elapsed(&timer));
}

struct kd_search_stat {
   const float *pt;    // query
   unsigned cnt;
   unsigned nn;
   float nn_dist;
};

void kd_search_node (const kd_node_t *node, struct kd_search_stat *stat, float d2b);

static inline void kd_search_leaf_or_node (union kd_leaf_or_node lon,
                                           struct kd_search_stat *stat,
                                           float d2b) {

   if (kd_is_leaf(lon)) {
      float l = l2sqr(stat->pt, means + dim * lon.leaf, dim);
      stat->cnt++;
      if (l < stat->nn_dist) {
         stat->nn = lon.leaf;
         stat->nn_dist = l;
      }
   }
   else { 
      kd_search_node(lon.node, stat, d2b);
   }
}

void kd_search_node (const kd_node_t *node, struct kd_search_stat *stat, float d2b) {

   unsigned cd = node->cut_dim;
   float cv = node->cut_val;
   const float *pt = stat->pt;
   float cut_diff = pt[cd] - cv;
   float box_diff;

   if (cut_diff < 0) {
      kd_search_leaf_or_node(node->left, stat, d2b);

      box_diff = node->lower - pt[cd];
      if (box_diff < 0) {
         box_diff = 0;
      }
      d2b = d2b + sqr(cut_diff) - sqr(box_diff);

      if (d2b < stat->nn_dist) {
         kd_search_leaf_or_node(node->right, stat, d2b);
      }
   }
   else {
      kd_search_leaf_or_node(node->right, stat, d2b);

      float box_diff = pt[cd] - node->upper;
      if (box_diff < 0) {
         box_diff = 0;
      }

      d2b = d2b + sqr(cut_diff) - sqr(box_diff);

      if (d2b < stat->nn_dist) {
         kd_search_leaf_or_node(node->left, stat, d2b);
      }
   }
}

// lookup is readonly
unsigned kd_search (const float *pt, unsigned *cnt) {

   unsigned d;
   float d2b = 0;
   for (d = 0; d < dim; d++) {
      if (pt[d] < kd_bnds.lo[d]) d2b += sqr(kd_bnds.lo[d] - pt[d]);
      else if (pt[d] > kd_bnds.hi[d]) d2b += sqr(pt[d] - kd_bnds.hi[d]);
   }

   struct kd_search_stat stat;
   stat.pt = pt;
   stat.cnt = 0;
   stat.nn = 0;
   stat.nn_dist = FLT_MAX;

   kd_search_node(kd_root, &stat, d2b);

   *cnt = stat.cnt;

   return stat.nn;
}

#endif // USE_KD_TREE
