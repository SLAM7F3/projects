// =========================================================================
// Ann_Analyzer class member function definitions
// =========================================================================
// Last modified on 1/15/09; 1/17/09; 9/3/09; 12/4/10; 5/31/13
// =========================================================================

#include <fstream>
#include "kdtree/ann_analyzer.h"
#include "math/genvector.h"

using std::cerr;
using std::cout;
using std::endl;
using std::ifstream;
using std::ios;
using std::istream;
using std::ostream;
using std::pair;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void ann_analyzer::initialize_member_objects()
{
   k = 1;			// number of nearest neighbors
   eps = 0;			// error bound
   maxPts = 1000;		// maximum number of data points
   nPts=-1;

   dataPts=NULL;
   queryPt=NULL;
   nnIdx=NULL;
   sqrd_dists=NULL;
   kdtree_ptr=NULL;
   
   dataIn=NULL;
   queryIn=NULL;

//   int max_pts_to_visit=200;
//   annMaxPtsVisit(max_pts_to_visit);
}		 

void ann_analyzer::allocate_member_objects()
{
   queryPt = annAllocPt(dim);          
   nnIdx = new ANNidx[k];   	// nearest neighbor indices
   sqrd_dists = new ANNdist[k];	// nearest neighbor distances
}

// ---------------------------------------------------------------------
ann_analyzer::ann_analyzer(int dim)
{
   initialize_member_objects();
   this->dim=dim;

   allocate_member_objects();
}

// ---------------------------------------------------------------------
ann_analyzer::ann_analyzer(int dim,int n_nearest_neighbors,int maxPts,
                           double eps)
{
   initialize_member_objects();

   this->dim=dim;
   k=n_nearest_neighbors;
   this->maxPts=maxPts;
   this->eps=eps;

   allocate_member_objects();
}

// ---------------------------------------------------------------------
// Copy constructor:

ann_analyzer::ann_analyzer(const ann_analyzer& a)
{
//   cout << "inside ann_analyzer copy constructor, this(ann_analyzer) = " << this << endl;
   initialize_member_objects();
   allocate_member_objects();
   docopy(a);
}

ann_analyzer::~ann_analyzer()
{
   delete nnIdx;
   delete sqrd_dists;
   delete kdtree_ptr;

   if (dataPts != NULL) annDeallocPts(dataPts);
   annClose();
}

// ---------------------------------------------------------------------
void ann_analyzer::docopy(const ann_analyzer& a)
{
//   cout << "inside ann_analyzer::docopy()" << endl;
//   cout << "this = " << this << endl;
}

// Overload = operator:

ann_analyzer& ann_analyzer::operator= (const ann_analyzer& a)
{
//   cout << "inside ann_analyzer::operator=" << endl;
//   cout << "this(ann_analyzer) = " << this << endl;
   if (this==&a) return *this;
   docopy(a);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const ann_analyzer& a)
{
   outstream << endl;
//   outstream << "Ann_Analyzer ID = " << e.ID << endl;
   
   return outstream;
}

// =========================================================================
// Kdtree initialization member functions
// =========================================================================

// Member function readPt reads in a single ANNpoint (false on EOF)

bool ann_analyzer::readPt(istream &in, ANNpoint p)  
{
   for (int i = 0; i < dim; i++) 
   {
      if (!(in >> p[i])) return false;
   }
   return true;
}

// ---------------------------------------------------------------------
// Member function printPt outputs a single ANNpoint

void ann_analyzer::printPt(ostream &out, ANNpoint p)			
{
   out << "(" << p[0];
   for (int i = 1; i < dim; i++) 
   {
      out << ", " << p[i];
   }
   out << ")" << endl;
}

// ---------------------------------------------------------------------
void ann_analyzer::getArgs(int argc, char **argv)
{
   static ifstream dataStream;	      // data file stream
   static ifstream queryStream;	      // query file stream

   if (argc <= 1) 
   {		      // no arguments
      cerr << "Usage:\n\n"
           << "  ann_sample [-d dim] [-max m] [-nn k] [-e eps] [-df data]"
         " [-qf query]\n\n"
           << "  where:\n"
           << "    dim      dimension of the space (default = 2)\n"
           << "    m        maximum number of data points (default = 1000)\n"
           << "    k        number of nearest neighbors per query (default 1)\n"
           << "    eps      the error bound (default = 0.0)\n"
           << "    data     name of file containing data points\n"
           << "    query    name of file containing query points\n\n"
           << " Results are sent to the standard output.\n"
           << "\n"
           << " To run this demo use:\n"
           << "    ann_sample -df data.pts -qf query.pts\n";
      exit(0);
   }

   int i = 1;
   while (i < argc) 
   {			// read arguments
      if (!strcmp(argv[i], "-d")) 
      {	// -d option
         dim = atoi(argv[++i]);		// get dimension to dump
      }
      else if (!strcmp(argv[i], "-max")) 
      {   // -max option
         maxPts = atoi(argv[++i]);	     // get max number of points
      }
      else if (!strcmp(argv[i], "-nn")) 
      {    // -nn option
         k = atoi(argv[++i]);		     // get number of near neighbors
      }
      else if (!strcmp(argv[i], "-e")) 
      {		// -e option
         sscanf(argv[++i], "%lf", &eps);		// get error bound
      }
      else if (!strcmp(argv[i], "-df")) 
      {		// -df option
         dataStream.open(argv[++i], ios::in);// open data file
         if (!dataStream) {
            cerr << "Cannot open data file" << endl;
            exit(1);
         }
         dataIn = &dataStream;		 // make this the data stream
      }
      else if (!strcmp(argv[i], "-qf")) 
      {		// -qf option
         queryStream.open(argv[++i], ios::in);// open query file
         if (!queryStream) 
         {
            cerr << "Cannot open query file" << endl;
            exit(1);
         }
         queryIn = &queryStream;	// make this query stream
      }
      else 
      {			       // illegal syntax
         cerr << "Unrecognized option." << endl;
         exit(1);
      }
      i++;
   }

   if (dataIn == NULL || queryIn == NULL) 
   {
      cerr << "-df and -qf options must be specified" << endl;
      exit(1);
   }
}

// ---------------------------------------------------------------------
// Member function load_data_points

int ann_analyzer::load_data_points(const vector<feature_pair>& data)
{
//   cout << "inside ann_analyzer::load_data_points()" << endl;
   
   nPts=basic_math::min(int(data.size()),maxPts);

// Allocate dataPts just once to hold maximal number of points:

   if (dataPts==NULL)
   {
      dataPts = annAllocPts(maxPts, dim);  // allocate data points      
   }
 
   for (int i=0; i<nPts; i++)
   {
      for (int d=0; d<dim; d++)
      {
         (dataPts[i])[d]=data[i].second->get(d);
      } 
   } // loop over index i labeling data points

   reset_kdtree();

   return nPts;
}

// ---------------------------------------------------------------------
void ann_analyzer::reset_kdtree()
{
//   cout << "inside ann_analyzer::reset_kdtree()" << endl;
   delete kdtree_ptr;
   kdtree_ptr=NULL;

//   cout << "Before instantiating new kdtree_ptr" << endl;
//   cout << "this = " << this << endl;
//   cout << "dataPts = " << dataPts << endl;
//   cout << "nPts = " << nPts << endl;
//   cout << "dim = " << dim << endl;

   kdtree_ptr = new ANNkd_tree(	     
      dataPts,			     // the data points
      nPts,			     // number of points
      dim);			     // dimension of space

//   cout << "kdtree_ptr = " << kdtree_ptr << endl;
}

// =========================================================================
// Kdtree search member functions
// =========================================================================

// Member function find_nearest_neighbors takes in descriptor
// descriptor *D_ptr whose dimension is assumed to equal member
// variable dim.  This loop returns the k closest data points to the
// input *D_ptr.

void ann_analyzer::find_nearest_neighbors(descriptor* D_ptr,bool print_flag)
{
//   cout << "inside ann_analyzer::find_nearest_neighbors()" << endl;

//   queryPt=D_ptr->get_e_ptr();
//   cout << "Query point = " << endl;
//   printPt(cout, queryPt);

//   kdtree_ptr->annkSearch(	     // search
   kdtree_ptr->annkPriSearch(	     // search
      D_ptr->get_e_ptr(),	     // query point
      k,			     // number of near neighbors
      nnIdx,			     // nearest neighbors (returned)
      sqrd_dists,		     // squared distances (returned)
      eps);			     // error bound

// Print nearest neighbor results for current query point:

   if (print_flag)
   {
      cout << "\tNN:\tIndex\tDistance" << endl;
      for (int i = 0; i < k; i++) 
      {		       
         double curr_dist=sqrt(sqrd_dists[i]);
         cout << "\t" << i << "\t" << nnIdx[i] << "\t" << curr_dist
              << endl;
      }
   }

}

// ---------------------------------------------------------------------
void ann_analyzer::find_nearest_neighbors()
{
   descriptor* D_ptr=new descriptor(dim);

   while (readPt(*queryIn, queryPt)) 
   {						       
      for (int d=0; d<dim; d++)
      {
         D_ptr->put(d,queryPt[d]);
      }
      find_nearest_neighbors(D_ptr);
   }

   delete D_ptr;
}

// --------------------------------------------------------------------------
// Member function match_feature_descriptor takes in 128-dimensional
// descriptor *input_D_ptr.  It first retrieves all keys and
// descriptors corresponding to image specified by input parameter
// image_index from input currimage_feature_info.  It then computes
// the squared distances d1_sqr and d2_sqr between the closest and
// next-to-closest candidate partner feature descriptors and the input
// descriptor.  Following D. Lowe and N. Snavely, we accept the
// closest candidate feature as a genuine match if d1/d2 <
// sqrt(sqrd_max_ratio).  Otherwise, this method returns NULL entries
// for the pair of descriptor ptrs.

ann_analyzer::feature_pair ann_analyzer::match_feature_descriptor(
   descriptor* input_D_ptr,const vector<feature_pair>& currimage_feature_info,
   double sqrd_max_ratio)
{
//   cout << "inside ann_analyzer::match_feature_descriptor()" << endl;
//   cout << "*input_D_ptr = " << *input_D_ptr << endl;

//   find_nearest_neighbors(input_D_ptr);

   kdtree_ptr->annkPriSearch(	     // search
      input_D_ptr->get_e_ptr(),	     // query point
      k,			     // number of near neighbors
      nnIdx,			     // nearest neighbors (returned)
      sqrd_dists,		     // squared distances (returned)
      eps);			     // error bound

   feature_pair matching_feature_pair(
      static_cast<descriptor*>(NULL),
      static_cast<descriptor*>(NULL));

//   cout << "sqrd_dists[0] = " << sqrd_dists[0]
//       << " sqrd_dists[1] = " << sqrd_dists[1] << endl;

   if (sqrd_dists[0]/sqrd_dists[1] < sqrd_max_ratio)
   {
      int nearest_neighbor_index=nnIdx[0];
      matching_feature_pair.first=
         currimage_feature_info[nearest_neighbor_index].first;
      matching_feature_pair.second=
         currimage_feature_info[nearest_neighbor_index].second;
   }

   return matching_feature_pair;
}

// --------------------------------------------------------------------------
// We have tried to streamline this next overloaded version of
// match_feature_descriptor as much as possible so that it runs as
// fast as possible:

bool ann_analyzer::match_feature_descriptor(
   descriptor* input_D_ptr,const vector<feature_pair>& currimage_feature_info,
   double sqrd_max_ratio,feature_pair& matching_feature_pair)
{
//   cout << "inside ann_analyzer::match_feature_descriptor()" << endl;
//   cout << "*input_D_ptr = " << *input_D_ptr << endl;

   kdtree_ptr->annkSearch(	     // search
//   kdtree_ptr->annkPriSearch(	     // search
      input_D_ptr->get_e_ptr(),	     // query point
      k,			     // number of near neighbors
      nnIdx,			     // nearest neighbors (returned)
      sqrd_dists,		     // squared distances (returned)
      eps);			     // error bound

//   cout << "sqrd_dists[0] = " << sqrd_dists[0]
//       << " sqrd_dists[1] = " << sqrd_dists[1] << endl;
//   double ratio = sqrd_dists[0]/sqrd_dists[1];

   if (sqrd_dists[0]/sqrd_dists[1] < sqrd_max_ratio)
   {
//      cout << "ratio = " << ratio << endl;

      int nearest_neighbor_index=nnIdx[0];
      matching_feature_pair.first=
         currimage_feature_info[nearest_neighbor_index].first;
      matching_feature_pair.second=
         currimage_feature_info[nearest_neighbor_index].second;
      return true;
   }
   else
   {
      return false;
   }
}
