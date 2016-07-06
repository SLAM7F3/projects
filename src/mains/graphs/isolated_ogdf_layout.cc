// =========================================================================
// Program ISOLATED_OGDF_LAYOUT parses a text file containing a
// bipartite graph edge list.  It then calls a graphical layout
// algorithm within the C++ Open Graph Drawing Framework (OGDF)
// library.  As of 9/1/09, we have found that only the Fast Multipole
// Multilevel layout algorithm can handle the 25K connected graph
// coming from the July 2009 MIT photo collect.  This program outputs
// the OGDF layout as a Graph Modeling Language (GML) text file which
// can be viewed using the yEd java graph tool.

// This particular main program does not depend upon any of Peter's
// code tree.  So Delsey can hopefully run this main program on any
// linux box which has a reasonable linux installation on it.

// =========================================================================
// Last updated on 9/1/09; 9/10/09; 9/11/09
// =========================================================================

#include <map>
#include <vector>
#include <string>
#include <sstream>	// Needed for using string streams within 
#include <ogdf/energybased/FMMMLayout.h>
#include <ogdf/basic/Graph.h>
#include <ogdf/basic/graph_generators.h>
#include <ogdf/energybased/SpringEmbedderFR.h>
#include <ogdf/layered/SugiyamaLayout.h>

using namespace ogdf;
using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::hex;
using std::ifstream;
using std::map;
using std::string;
using std::ofstream;
using std::ostringstream;
using std::stringstream;
using std::vector;

// =========================================================================
// Method Openfile takes in a filename string along with an
// ofstream and opens up a filestream:

bool openfile(string filenamestr,ifstream& filestream)
{

// Ifstream constructor must take a C-style char* string argument
// rather than a C++ string class object:

   filestream.open(filenamestr.c_str(),ios::in);

   bool file_opened=true;
   if (!filestream)
   {
      cerr << "Cannot open " << filenamestr << " for input" << endl;
      file_opened=false;
   }
   return file_opened;
}

bool openfile(string filenamestr,ofstream& filestream)
{

   filestream.open(filenamestr.c_str(),ios::out);

   bool file_opened=true;
   if (!filestream)
   {
      cerr << "Method filefunc::openfile(string,ofstream) cannot open " 
           << filenamestr << " for output" << endl;
      file_opened=false;
   }
   return file_opened;
}

void closefile(string filenamestr,ifstream& filestream)
{
   filestream.close();
}

void closefile(string filenamestr,ofstream& filestream)
{
   filestream.close();
}

// ---------------------------------------------------------------------
// Method ReadInfile opens up a file with name specified by string
// variable filenamestr, reads in its contents into array line,
// returns the number of lines within the file within integer variable
// nlines, and then closes the input file.  If the requested file was
// not successfully read, this boolean method returns false.

bool ReadInfile(string filenamestr,vector<string>& line)
{
//         cout << "inside filefunc::ReadInfile(filenamestr,line)" << endl;

   ifstream currfile;
   bool file_successfully_opened=openfile(filenamestr,currfile);
   string currline;

   if (!file_successfully_opened)
   {
      cout << filenamestr 
           << " not successfully opened by method ReadInfile!" 
           << endl;
   }
   else
   {
      while (getline(currfile,currline,'\n'))
      {
         line.push_back(currline);
      }
   }
   closefile(filenamestr,currfile);
   
   return file_successfully_opened;
}


// ---------------------------------------------------------------------
// Methods prefix and suffix take in an inputstring which is assumed
// to be of the form "prefix.suffix".  These methods return the
// substrings before and after the dot within the inputstring.

string prefix(string inputstring,string separator)
{
   int last_dot_position = inputstring.find_last_of(
      separator,inputstring.size());
   return inputstring.substr(0,last_dot_position);
}

// ---------------------------------------------------------------------
// Method integer_to_hexadecimal takes in integer i along with the
// number of desired output hexadecimal digits.  It returns a string
// containing the hexadecimal format for the integer which is
// pre-padded with zeros.

string integer_to_hexadecimal(int i,int ndigits)
{
   char buffer[50];
   stringstream ss;

   ss << hex << i;
   ss >> buffer;
   string output=buffer;

   int initial_ndigits=output.size();
   for (int n=0; n<ndigits-initial_ndigits; n++)
   {
      output="0"+output;
   }
   return output;
}

// ---------------------------------------------------------------------
// Method RGB_to_RRGGBB_hex takes in r,g,b color values ranging
// between 0 and 1.  It first converts each color double into the
// integer interval [0,255].  It then converts each integer to its
// hexadecimal representation.  The concatenated hexadecimal version
// is returned.

string RGB_to_RRGGBB_hex(double r,double g,double b)
{
   int R=r*255;
   R=max(0,R);
   R=min(255,R);
   string R_hex=integer_to_hexadecimal(R,2);

   int G=g*255;
   G=max(0,G);
   G=min(255,G);
   string G_hex=integer_to_hexadecimal(G,2);

   int B=b*255;
   B=max(0,B);
   B=min(255,B);
   string B_hex=integer_to_hexadecimal(B,2);

//         cout << "R_hex = " << R_hex 
//              << " G_hex = " << G_hex
//              << " B_hex = " << B_hex << endl;

   string RRGGBB_hex=R_hex+G_hex+B_hex;
//         cout << "RRGGBB_hex = " << RRGGBB_hex << endl;
   return RRGGBB_hex;
}


// ---------------------------------------------------------------------
vector<string> decompose_string_into_substrings(
   const string& str,const string& delimiters)
{
   vector<string> tokens;

   // Skip delimiters at beginning.
   string::size_type lastPos = str.find_first_not_of(delimiters, 0);
   // Find first "non-delimiter".
   string::size_type pos     = str.find_first_of(delimiters, lastPos);

   while (string::npos != pos || string::npos != lastPos)
   {
      // Found a token, add it to the vector.
      tokens.push_back(str.substr(lastPos, pos - lastPos));
      // Skip delimiters.  Note the "not_of"
      lastPos = str.find_first_not_of(delimiters, pos);
      // Find next "non-delimiter"
      pos = str.find_first_of(delimiters, lastPos);
   }

   return tokens;
}

/*
vector<string> decompose_string_into_substrings(
   string initial_inputstring,string separator_chars)
{
//         cout << "inside stringfuncs::decompose_string_into_substrings()"
//              << endl;
         
//         const string whitespace(" \t\n");
   const string lowerletters="abcdefghijklmnopqrstuvwxyz";
   const string upperletters="ABCDEFGHIJKLMNOPQRSTUVWXYZ";
//         const string lowerletters="abcdfghijklmnopqrstuvwxyz";
//         const string upperletters="ABCDFGHIJKLMNOPQRSTUVWXYZ";
   const string extrachars="~!@#$%^&*()_|=<>?/';:[]{}";
   const string numerics("0123456789+-.");
   string allchars=numerics+lowerletters+upperletters+extrachars;

// First clear all entries within the substring array:

   vector<string> substring;
   vector<unsigned> char_posn;
   vector<unsigned> separator_posn;

   separator_posn.push_back(0);
   int i=0;
   unsigned int j,k;

// For reasons which we don't understand as of Oct 2007, the final
// substring always seems to be missing its final non-white character.
// So we append a dummy space to the initial input string:

   string inputstring=initial_inputstring+" ";

   do
   {
      j=inputstring.find_first_of(allchars,separator_posn[i]);
//            cout << "j = " << j << endl;
      if (j==string::npos) break;
      if (j < inputstring.length()) char_posn.push_back(j);

      k=inputstring.find_first_of(separator_chars,char_posn[i]+1);
//            cout << "k = " << k << endl;
      if (k==string::npos) k=inputstring.length()-1;
      if (k < inputstring.length()) separator_posn.push_back(k);
      i++;
   }
   while (separator_posn.back() < inputstring.length()-1 &&
          char_posn.back() < inputstring.length());
         
   for (int j=0; j<i; j++)
   {
      substring.push_back(inputstring.substr(
         char_posn[j],separator_posn[j+1]-char_posn[j]));
   }
   return substring;
}
*/

// =========================================================================
int main()
{

// Read in Delsey's edge list:

   string subdir="./delsey/";
   string filename;
//   string filename=subdir+"single_mode_test.csv";
//   string filename=subdir+"bipartite_test2.csv";
//   string filename=subdir+"orgs_giant.csv";
//   string filename=subdir+"orgs_docs.csv";
//   string filename=subdir+"perslocorg_docs.csv";
   cout << "Enter name of input edge list file:" << endl;
   cin >> filename;
   filename=subdir+filename;

   vector<string> text_line;
   ReadInfile(filename,text_line);

   vector<string> first_node_ID,second_node_ID;
   for (int i=0; i<text_line.size(); i++)
   {
      vector<string> FS=decompose_string_into_substrings(text_line[i],",");
      first_node_ID.push_back(FS[0]);
      second_node_ID.push_back(FS[1]);

//      cout << "first_node = " << first_node_ID.back()
//           << " 2nd node = " << second_node_ID.back() << endl;
   }

// Instantiate OGDF graph and node_map:

   typedef std::map<string,node> NODE_MAP;
   NODE_MAP node_map;

   Graph G;

//   GraphAttributes GA(G);
//   GraphAttributes GA(
//      G,GraphAttributes::nodeLabel);
   GraphAttributes GA(
      G,
      GraphAttributes::nodeGraphics | GraphAttributes::edgeGraphics |
      GraphAttributes::nodeLabel | GraphAttributes::nodeColor | 
      GraphAttributes::edgeColor | GraphAttributes::edgeStyle | 
      GraphAttributes::nodeStyle | GraphAttributes::nodeTemplate);

   double r=1;
   double g=0;
   double b=0;
   string red_color="#"+RGB_to_RRGGBB_hex(r,g,b);

   r=0;
   g=0;
   b=1;
   string blue_color="#"+RGB_to_RRGGBB_hex(r,g,b);

   node node1,node2;
   int n_edges=first_node_ID.size();
   for (int e=0; e<n_edges; e++)
   {
      string curr_ID=first_node_ID[e];
      NODE_MAP::iterator iter1=node_map.find(curr_ID);
      if (iter1==node_map.end())
      {
         node1=G.newNode();
         node_map[curr_ID]=node1;
         cout << "curr_ID = " << curr_ID << endl;
         GA.labelNode(node1)=String(curr_ID.c_str());

//         string color_prefix=prefix(curr_ID,"0123456789");
         string color_prefix=curr_ID.substr(0,1);

//         if (color_prefix=="R")
         if (color_prefix=="E")
         {
            GA.colorNode(node1)=String(red_color.c_str());
         }
//         else if (color_prefix=="B")
         else if (color_prefix=="D")
         {
            GA.colorNode(node1)=String(blue_color.c_str());
         }
      }
      else
      {
         node1=iter1->second;
      }
      
      curr_ID=second_node_ID[e];
      NODE_MAP::iterator iter2=node_map.find(curr_ID);
      if (iter2==node_map.end())
      {
         node2=G.newNode();
         node_map[curr_ID]=node2;
//         cout << "curr_ID = " << curr_ID << endl;
         GA.labelNode(node2)=String(curr_ID.c_str());

//         string color_prefix=prefix(curr_ID,"0123456789");
         string color_prefix=curr_ID.substr(0,1);
//         cout << "color_prefix2 = " << color_prefix << endl;

//         if (color_prefix=="R")
         if (color_prefix=="E")
         {
            GA.colorNode(node2)=String(red_color.c_str());
         }
//         else if (color_prefix=="B")
         else if (color_prefix=="D")
         {
            GA.colorNode(node2)=String(blue_color.c_str());
         }
      }
      else
      {
         node2=iter2->second;
      }
      
      G.newEdge(node1,node2);
   }

   cout << "node_map.size() = " << node_map.size() << endl;

   node v;
   forall_nodes(v,G)
      {
         GA.width(v) = GA.height(v) = 10.0;
      }

// Fast Multipole Multilevel Layout algorithm

   cout << "Computing Fast Multiple Multilevel layout:" << endl;

   FMMMLayout fmmm;
   fmmm.useHighLevelOptions(true);
   fmmm.unitEdgeLength(15.0); 
   fmmm.newInitialPlacement(true);
   fmmm.qualityVersusSpeed(FMMMLayout::qvsGorgeousAndEfficient);
   fmmm.call(GA);

   GA.writeGML("graph_layout.gml");

   ofstream outstream;
   string output_filename="nodes_layout.txt";
   openfile(output_filename,outstream);

   forall_nodes(v,G)
      {
         cout << string((GA.labelNode(v)).cstr()) 
              << "," << GA.x(v) << "," << GA.y(v) << endl;
         outstream << string((GA.labelNode(v)).cstr()) 
                   << "," << GA.x(v) << "," << GA.y(v) << endl;
//         cout << "x = " << GA.x(v)
//              << " y = " << GA.y(v)
//              << " label = " << string((GA.labelNode(v)).cstr()) 
//              << endl;
      }

   closefile(output_filename,outstream);

}

