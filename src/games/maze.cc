// ==========================================================================
// maze class member function definitions
// ==========================================================================
// Last modified on 11/16/16; 11/17/16; 11/18/16; 11/21/16
// ==========================================================================

#include <iostream>
#include <string>
#include "color/colorfuncs.h"
#include "general/filefuncs.h"
#include "math/mathfuncs.h"
#include "games/maze.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "math/twovector.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::ofstream;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void maze::allocate_member_objects()
{
   n_cells = n_size * n_size;
   n_directions = 4;  // 2D
   nbits = n_directions;
   grid_ptr = new genmatrix(n_cells, n_cells);
   grid_problems_ptr = new genmatrix(n_cells, n_cells);
   soln_grid_ptr = new genmatrix(n_size, n_size);

   occupancy_grid = new genmatrix(2 * n_size - 1, 2 * n_size - 1);
   occupancy_state = new genvector(sqr(2*n_size-1));
   visited_turtle_cells = new genmatrix(2 * n_size - 1, 2 * n_size - 1);
   ImageSize = 512;
   if(n_size > 6)
   {
      ImageSize = 1024;
   }
   curr_legal_actions = new genvector(n_directions);
}		       

void maze::initialize_member_objects()
{
   direction.push_back(1); // up
   direction.push_back(2); // right
   direction.push_back(4); // down
   direction.push_back(8); // left

// Load cell_decomposition vector with (px,py) duples
// corresponding to p = 0 --> n_cells - 1:

   for(int q = 0; q < n_cells; q++)
   {
      int p = q;
      int py = p / n_size;
      p -= n_size * py;
      int px = p;
      cell_decomposition.push_back(DUPLE(px,py));
   }

// Load occupancy_cell_decomposition vector with (px,py) duples
// corresponding to p = 0 --> occupancy_grid.mdim * occupancy_grid.ndim - 1:

   for(unsigned int q = 0; 
       q < occupancy_grid->get_mdim() * occupancy_grid->get_ndim(); q++)
   {
      int p = q;
      int py = p / occupancy_grid->get_ndim();
      p -= occupancy_grid->get_ndim() * py;
      int px = p;
      occupancy_cell_decomposition.push_back(DUPLE(px,py));
   }

   turtle_cell = -1;
   turtle_p_start = 0;
}		       

// ---------------------------------------------------------------------
maze::maze(int n_size)
{
   this->n_size = n_size;

   allocate_member_objects();
   initialize_member_objects();
}

// Copy constructor:

maze::maze(const maze& T)
{
//   docopy(T);
}

// ---------------------------------------------------------------------
maze::~maze()
{
   delete grid_ptr;
   delete grid_problems_ptr;
   delete soln_grid_ptr;
   delete curr_legal_actions;
   delete occupancy_grid;
   delete occupancy_state;
   delete visited_turtle_cells;

   for(unsigned int t = 0; t < curr_maze_states.size(); t++)
   {
      delete curr_maze_states[t];
   }
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const maze& T)
{
   outstream << endl;
   return outstream;
}

// ==========================================================================

DUPLE maze::getDirection(int curr_dir)
{
   if(curr_dir == 0)  	   // up
   {
      return DUPLE(0,-1);
   }
   else if(curr_dir == 1)  // right
   {
      return DUPLE(1,0);
   }
   else if(curr_dir == 2)  // down
   {
      return DUPLE(0,1);
   }
   else if(curr_dir == 3)  // lefet
   {
      return DUPLE(-1,0);
   }
   else
   {
      cout << "Error in maze::getDirection()" << endl;
      cout << "curr_dir = " << curr_dir << endl;
      exit(-1);
   }
}

// ---------------------------------------------------------------------
bool maze::IsDirValid(int px, int py, int curr_dir)
{
   DUPLE currDirection = getDirection(curr_dir);
   int qx = px + currDirection.first;
   int qy = py + currDirection.second;
   if(qx < 0 || qx >= n_size || qy < 0 || qy >= n_size)
   {
      return false;
   }
   return true;
}

// ---------------------------------------------------------------------
int maze::get_neighbor(int p, int curr_dir)
{
   int px = cell_decomposition[p].first;
   int py = cell_decomposition[p].second;
   if(!IsDirValid(px,py,curr_dir)) return -1;
   DUPLE currDirection = getDirection(curr_dir);
   int qx = px + currDirection.first;
   int qy = py + currDirection.second;
   if(qx < 0 || qx >= n_size || qy < 0 || qy >= n_size)
   {
      return -1;
   }
   else
   {
      return get_cell(qx,qy);
   }
}

// ---------------------------------------------------------------------
string maze::get_cell_bitstr(int px, int py)
{
   int cell_value = grid_ptr->get(px,py);
   char cell_char = stringfunc::ascii_integer_to_char(cell_value);
   return stringfunc::byte_bits_rep(cell_char,nbits);
}

// ---------------------------------------------------------------------
int maze::get_direction_from_p_to_q(int p, int q)
{
   int px = cell_decomposition[p].first;
   int py = cell_decomposition[p].second;
   int qx = cell_decomposition[q].first;
   int qy = cell_decomposition[q].second;
   
   int delta_x = qx - px;
   int delta_y = qy - py;

   if(delta_x == 0 && delta_y == -1)
   {
      return 0; // up
   }
   else if(delta_x == 1 && delta_y == 0)
   {
      return 1; // right
   }
   else if(delta_x == 0 && delta_y == 1)
   {
      return 2; // down
   }
   else if(delta_x == -1 && delta_y == 0)
   {
      return 3; // left
   }
   else
   {
      cout << "Error in get_direction_from_p_to_q()" << endl;
      cout << "delta_x = " << delta_x << " delta_y = " << delta_y
           << endl;
      exit(-1);
   }
}

// ---------------------------------------------------------------------
void maze::remove_wall(int p, int curr_dir)
{
   int px = cell_decomposition[p].first;
   int py = cell_decomposition[p].second;
   string cell_bitstr = get_cell_bitstr(px, py);

// Reset bit labeled by curr_dir to zero:

   cell_bitstr.replace(nbits-1-curr_dir, 1, "0");
   double new_cell_value = stringfunc::bits_rep_to_integer(cell_bitstr);
   grid_ptr->put(px, py, new_cell_value);
}

// ---------------------------------------------------------------------
// Member function wall_exists() takes in a grid cell ID and direction
// label.  It retrieves the bit-string for the specified cell.  If the
// bit-string contains a "1" in the bit location corresponding to
// curr_dir, this boolean method returns true.

bool maze::wall_exists(int p, int curr_dir)
{
   int px = cell_decomposition[p].first;
   int py = cell_decomposition[p].second;
   string cell_bitstr = get_cell_bitstr(px, py);

   string curr_bitstr = cell_bitstr.substr(nbits-1-curr_dir,1);
   return (curr_bitstr == "1");
}

// ---------------------------------------------------------------------
// Member function get_cell_neighbors() returns the IDs for all
// n_directions neighbors of input cell p (regardless of whether
// they're accessible from p or blocked by a wall).

vector<int> maze::get_cell_neighbors(int p)
{
   vector<int> cell_neighbor_ids;
   for(int curr_dir = 0; curr_dir < n_directions; curr_dir++)
   {
      int curr_neighbor = get_neighbor(p, curr_dir);
      cell_neighbor_ids.push_back(curr_neighbor);
   }
   return cell_neighbor_ids;
}

// ---------------------------------------------------------------------
vector<int> maze::get_unvisited_neighbors(int p)
{
   vector<int> unvisited_neighbors;
   
   vector<int> cell_neighbor_ids = get_cell_neighbors(p);
   for(unsigned int n = 0; n < cell_neighbor_ids.size(); n++)
   {
      int cell_neighbor_id = cell_neighbor_ids[n];
      if(cell_neighbor_id < 0) continue;
      if(visited_cell[cell_neighbor_id] == false)
      {
         unvisited_neighbors.push_back(cell_neighbor_id);
      }
   }
   return unvisited_neighbors;
}

// ---------------------------------------------------------------------
int maze::n_visited_cells() const
{
   int n_visited_cells = 0;
   for(unsigned int n = 0; n < visited_cell.size(); n++)
   {
      n_visited_cells += visited_cell[n];
   }
   return n_visited_cells;
}

// ---------------------------------------------------------------------
void maze::init_grid()
{
   for(int py = 0; py < n_size; py++)
   {
      for(int px = 0; px < n_size; px++)
      {
         grid_ptr->put(px,py,15); // All cells start with 4 walls
      }
   }
}

// ---------------------------------------------------------------------
void maze::print_grid() const
{
   for(int py = 0; py < n_size; py++)
   {
      for(int px = 0; px < n_size; px++)
      {
         cout << grid_ptr->get(px,py) << "   " << flush;
      }
      cout << endl;
   }
   cout << endl;
}

// ---------------------------------------------------------------------
void maze::print_visited_cells() const
{
   for(int py = 0; py < n_size; py++)
   {
      for(int px = 0; px < n_size; px++)
      {
         int p = py * n_size + px;
         cout << visited_cell[p] << "   " << flush;
      }
      cout << endl;
   }
   cout << endl;
}

// ---------------------------------------------------------------------
void maze::print_visited_cell_stack() const
{
   for(unsigned int v = 0; v < visited_cell_stack.size(); v++)
   {
      cout << "v = " << v << " visited_cell_stack = " 
           << visited_cell_stack[v] 
           << endl;
   }
}

// ---------------------------------------------------------------------
void maze::set_solution_path() 
{
   soln_path.clear();
   soln_path.push_back(0);

   for(int v = visited_cell_stack.size()-1; v >= 0; v--)
   {
      soln_path.push_back(visited_cell_stack[v]);
   }
}

const vector<int>& maze::get_solution_path() const
{
   return soln_path;
}

int maze::get_solution_path_moves() const
{
   return soln_path.size();
}

void maze::print_solution_path() const
{
   cout << "Solution path:" << endl;
   for(unsigned int v = 0; v < soln_path.size(); v++)
   {
      int p = soln_path[v];
      int px = cell_decomposition[p].first;
      int py = cell_decomposition[p].second;
      
      cout << "   v = " << v << " Cell ID:" << p
           << " px = " << px << " py = " << py 
           << endl;
   }
}

// ---------------------------------------------------------------------
// Member function generate_maze() implements a depth-first search
// method for creating a maze.  It follows the algorithm given in
// https://en.wikipedia.org/wiki/Maze_generation_algorithm . 

void maze::generate_maze()
{
//   cout << "inside generate_maze()" << endl;

   turtle_value = 1.0;
   wall_value = -1.0;

   init_grid();

// Set all visited cell flags to false:

   visited_cell.clear();
   visited_cell_stack.clear();
   for(int p = 0; p < n_cells; p++)
   {
      visited_cell.push_back(false);
   }

// Start maze generation at some location within grid:

//   int p = 0;
   int p = n_cells-1;  // OK
//   int p = mathfunc::getRandomInteger(n_cells);

   visited_cell[p] = true;

   while(n_visited_cells() < n_cells)
   {
      vector<int> unvisited_neighbor_ids = get_unvisited_neighbors(p);
      int n_unvisited_neighbors = unvisited_neighbor_ids.size();
      if(n_unvisited_neighbors > 0)
      {
         int q = unvisited_neighbor_ids[
            mathfunc::getRandomInteger(n_unvisited_neighbors)];
         visited_cell_stack.push_back(p);

// Remove walls between p and q:
         
         int curr_dir = get_direction_from_p_to_q(p, q);
         remove_wall(p, curr_dir);
         remove_wall(q, (curr_dir+2)%4);

         p = q;
         visited_cell[p] = true;
      }
      else
      {
         if(visited_cell_stack.size() > 0)
         {
            p = visited_cell_stack.back();
            visited_cell_stack.pop_back();
         }
      } // n_unvisited_neighbors > 0 conditional

      if(p == 0)
      {
         set_solution_path();
      }
   } // n_visited_cells < n_cells while loop
}

// ==========================================================================
// Drawing member functions
// ==========================================================================

// Bitmap note: x = y = 0 corresponds to BOTTOM left corner of output
// image rather than to TOP left corner!

void maze::DrawLine(unsigned char* img, int x1, int y1, int x2, int y2 ,
                    int R, int G, int B)
{
   if ( x1 == x2 )
   {
      // vertical line
      for ( int y = y1; y < y2; y++ )
      {
         if ( x1 >= ImageSize || y >= ImageSize ) continue;
         int i = 3 * ( y * ImageSize + x1 );

         img[ i + 2 ] = R;
         img[ i + 1 ] = G;
         img[ i + 0 ] = B;
      }
   }

   if ( y1 == y2 )
   {
      // horizontal line
      for ( int x = x1; x < x2; x++ )
      {
         if ( y1 >= ImageSize || x >= ImageSize ) continue;
         int i = 3 * ( y1 * ImageSize + x );

         img[ i + 2 ] = R;
         img[ i + 1 ] = G;
         img[ i + 0 ] = B;
      }
   }
}

void maze::DrawPoint(unsigned char* img, const twovector& V,
                     int R, int G, int B)
{
   int px = V.get(0);
   int py = V.get(1);
   if(px < 0 || px >= ImageSize || py < 0 || py >= ImageSize) return;
   int p = 3 * (py * ImageSize + px);
   img[ p + 2 ] = R;
   img[ p + 1 ] = G;
   img[ p + 0 ] = B;
}

void maze::DrawLine(unsigned char* img, twovector& V1, twovector& V2,
                    int R, int G, int B)
{
   int px_start = basic_math::min(V1.get(0), V2.get(0));
   int px_stop = basic_math::max(V1.get(0), V2.get(0));

   int py_start = basic_math::min(V1.get(1), V2.get(1));
   int py_stop = basic_math::max(V1.get(1), V2.get(1));

   double delta_x = px_stop - px_start;
   double delta_y = py_stop - py_start;
   
   if(fabs(delta_x) > fabs(delta_y))
   {
      int n_steps = px_stop - px_start;
      for(int n = 0; n <= n_steps; n++)
      {
         double frac = double(n) / n_steps;
         int px = V1.get(0) + frac * (V2.get(0) - V1.get(0));
         int py = V1.get(1) + frac * (V2.get(1) - V1.get(1));
         twovector V(px,py);
         DrawPoint(img, V, R, G, B);
      }
   }
   else
   {
      int n_steps = py_stop - py_start;
      for(int n = 0; n <= n_steps; n++)
      {
         double frac = double(n) / n_steps;
         int px = V1.get(0) + frac * (V2.get(0) - V1.get(0));
         int py = V1.get(1) + frac * (V2.get(1) - V1.get(1));
         twovector V(px,py);
         DrawPoint(img, V, R, G, B);
      }
   }
}

// ---------------------------------------------------------------------
// Member function DrawArrow() draws an arrow pointing from (x1,y1) to
// (x2,y2):

void maze::DrawArrow( unsigned char* img, twovector& base, twovector& tip,
                     int R, int G, int B)
{
   twovector body = tip - base;
   twovector ehat = body.unitvector();

   double theta = 30 * PI/180;
   double cos_theta = cos(theta);
   double sin_theta = sin(theta);
   genmatrix Rot(2,2);
   Rot.put(0,0,cos_theta);
   Rot.put(1,0,sin_theta);
   Rot.put(0,1,-sin_theta);
   Rot.put(1,1,cos_theta);
   twovector epos_hat = Rot * ehat;
   twovector eneg_hat = Rot.transpose() * ehat;

   twovector slant_neg_start = tip - 0.5 * body.magnitude() * epos_hat;   
   twovector slant_pos_start = tip - 0.5 * body.magnitude() * eneg_hat;

   DrawLine(img, base, tip, R, G, B);
   DrawLine(img, slant_neg_start, tip, R, G, B);
   DrawLine(img, slant_pos_start, tip, R, G, B);
}

// ---------------------------------------------------------------------
// Member function DrawCellArrow() takes in cell coords (px,py) where
// (0,0) corresponds to the cell in the UPPER LEFT corner of the maze.
// It also takes in direction d = 0, 1, 2, 3 [up, right, bottom,
// left].  DrawCellArrow places a colored arrow in the center of the
// specified cell which points in the specified direction.

void maze::DrawCellArrow(unsigned char* img, int px, int py, int direction,
                         int R, int G, int B)
{
   double CellSize = ImageSize / n_size;

   twovector cell_midpoint(px + 0.5, py + 0.5);
   cell_midpoint *= CellSize;

   twovector fhat;
   if(direction == 0)
   {
      fhat = twovector(0,-1);
   }
   else if(direction == 1)
   {
      fhat = twovector(1,0);
   }
   else if(direction == 2)
   {
      fhat = twovector(0,1);
   }
   else if(direction == 3)
   {
      fhat = twovector(-1,0);
   }
   
   twovector base = cell_midpoint - 0.25 * fhat * CellSize;
   twovector tip = cell_midpoint + 0.25 * fhat * CellSize;

   DrawArrow(img, base, tip, R, G, B);
}

// ---------------------------------------------------------------------
void maze::DrawCellX(unsigned char* img, int px, int py, int R, int G, int B)
{
   double CellSize = ImageSize / n_size;

   twovector cell_midpoint(px + 0.5, py + 0.5);
   cell_midpoint *= CellSize;

   twovector fhat(1,1);
   fhat = fhat.unitvector();
   twovector base = cell_midpoint - 0.25 * fhat * CellSize;
   twovector tip = cell_midpoint + 0.25 * fhat * CellSize;
   DrawLine(img, base, tip, R, G, B);

   fhat = twovector(1,-1);
   fhat = fhat.unitvector();
   base = cell_midpoint - 0.25 * fhat * CellSize;
   tip = cell_midpoint + 0.25 * fhat * CellSize;
   DrawLine(img, base, tip, R, G, B);
}
// ---------------------------------------------------------------------
void maze::FillCell(unsigned char* img, int px, int py, int R, int G, int B)
{
   double CellSize = ImageSize / n_size;

   int qx_lo = px * CellSize;
   int qx_hi = (px + 1 ) * CellSize;
   int qy_lo = py * CellSize;
   int qy_hi = (py + 1 ) * CellSize;

   for(int qy = qy_lo; qy < qy_hi; qy++)
   {
      for(int qx = qx_lo; qx < qx_hi; qx++)
      {
         int q = 3 * (qy * ImageSize + qx);
         img[ q + 2 ] = R;
         img[ q + 1 ] = G;
         img[ q + 0 ] = B;
      }
   }
}

// ---------------------------------------------------------------------
void maze::RenderMaze( unsigned char* img )
{
   int CellSize = ImageSize / n_size;
   
   for ( int py = 0; py < n_size; py++ )
   {
      for ( int px = 0; px < n_size; px++ )
      {
         string bit_str = get_cell_bitstr(px, py);

         int npx = px * CellSize;
         int npy = py * CellSize;

         int R = 255;
         int G = 255;
         int B = 255;

         if(bit_str.substr(3,1) == "1")
            DrawLine( img, npx, npy, npx + CellSize + 1, npy, R, G, B);
         if (bit_str.substr(2,1) == "1")
            DrawLine(img, npx+CellSize, npy, npx+CellSize, npy+CellSize+1,
                     R, G, B);
         if (bit_str.substr(1,1) == "1")
            DrawLine(img, npx, npy+CellSize, npx+CellSize+1, npy+CellSize,
                     R, G, B);
         if (bit_str.substr(0,1) == "1")
            DrawLine(img, npx, npy, npx, npy+CellSize+1, R, G, B);
      }
   }
}

#if defined( __GNUC__ )
# define GCC_PACK(n) __attribute__((packed,aligned(n)))
#else
# define GCC_PACK(n) __declspec(align(n))
#endif // __GNUC__

#pragma pack(push, 1)
struct GCC_PACK( 1 ) sBMPHeader
{
   // BITMAPFILEHEADER
   unsigned short bfType;
   uint32_t bfSize;
   unsigned short bfReserved1;
   unsigned short bfReserved2;
   uint32_t bfOffBits;
   // BITMAPINFOHEADER
   uint32_t biSize;
   uint32_t biWidth;
   uint32_t biHeight;
   unsigned short biPlanes;
   unsigned short biBitCount;
   uint32_t biCompression;
   uint32_t biSizeImage;
   uint32_t biXPelsPerMeter;
   uint32_t biYPelsPerMeter;
   uint32_t biClrUsed;
   uint32_t biClrImportant;
};
#pragma pack(pop)

// ---------------------------------------------------------------------
void maze::SaveBMP(string FileName, const void* RawBGRImage, 
                   int Width, int Height )
{
   sBMPHeader Header;

   int ImageSize = Width * Height * 3;

   Header.bfType = 0x4D * 256 + 0x42;
   Header.bfSize = ImageSize + sizeof( sBMPHeader );
   Header.bfReserved1 = 0;
   Header.bfReserved2 = 0;
   Header.bfOffBits = 0x36;
   Header.biSize = 40;
   Header.biWidth = Width;
   Header.biHeight = Height;
   Header.biPlanes = 1;
   Header.biBitCount = 24;
   Header.biCompression = 0;
   Header.biSizeImage = ImageSize;
   Header.biXPelsPerMeter = 6000;
   Header.biYPelsPerMeter = 6000;
   Header.biClrUsed = 0;
   Header.biClrImportant = 0;

   ofstream File( FileName.c_str(), std::ios::out | std::ios::binary );
   File.write( ( const char* )&Header, sizeof( Header ) );
   File.write( ( const char* )RawBGRImage, ImageSize );
   string unix_cmd = "convert -flip "+FileName+" "+FileName;
   sysfunc::unix_command(unix_cmd);
//   cout << "Exported " << FileName << endl;
}
 
// ---------------------------------------------------------------------
void maze::DisplayTrainedZerothLayerWeights(string output_subdir)
{
   for(unsigned int w = 0; w < wtwoDarray_ptrs.size(); w++)
   {
      twoDarray* wtwoDarray_ptr = wtwoDarray_ptrs[w];
      string basename = "trained_weights";
      bool display_qmap_flag = true;
      DrawMaze(w, output_subdir, basename, display_qmap_flag, wtwoDarray_ptr);
   } // loop over index w labeling zeroth layer weights
}

// ---------------------------------------------------------------------
void maze::DrawMaze(int counter, string output_subdir, string basename, 
                    bool display_qmap_flag, twoDarray* wtwoDarray_ptr)
{
   // prepare BGR image
   size_t DataSize = 3 * ImageSize * ImageSize;
   unsigned char* Img = new unsigned char[ DataSize ];

   memset( Img, 0, DataSize );

   if(wtwoDarray_ptr != NULL) color_cells(Img, wtwoDarray_ptr);

   // render maze on bitmap
   RenderMaze( Img );

   if(display_qmap_flag)
   {
      draw_max_Qmap(Img);
   }

   filefunc::add_trailing_dir_slash(output_subdir);
   filefunc::dircreate(output_subdir);
   string bmp_filename = output_subdir+basename+".bmp";

   string png_subdir = output_subdir+"pngs/";
   filefunc::dircreate(png_subdir);
   string png_filename = png_subdir+basename+stringfunc::integer_to_string(
      counter,3)+".png";
   SaveBMP( bmp_filename, Img, ImageSize, ImageSize );
   string unix_cmd = "convert "+bmp_filename+" "+png_filename;
   sysfunc::unix_command(unix_cmd);
   filefunc::deletefile(bmp_filename);

   // cleanup
   delete[]( Img );

   string banner="Exported maze to "+png_filename;
   outputfunc::write_banner(banner);
}

// ---------------------------------------------------------------------
// Member function compute_max_Qmap() loops over all entries within
// *qmap_ptr.  It extracts the turtle's occupancy grid coordinates
// from each state along with its direction.  This method stores 
// the direction corresponding to the maximum Q value for each maze
// grid cell in member STL map max_qmap.

void maze::compute_max_Qmap()
{
   max_qmap.clear();
   
   for(unsigned int s = 0; s < curr_maze_state_strings.size(); s++)
   {
      string curr_state_str = curr_maze_state_strings[s];
      for(int a = 0; a < n_directions; a++)
      {
         string state_action_str = curr_state_str+stringfunc::number_to_string(
            a);

         int turtle_cell = state_action_str.find("T");
         int turtle_direction = stringfunc::string_to_number(
            state_action_str.substr(state_action_str.size() - 1, 1));

         qmap_iter = qmap_ptr->find(state_action_str);
         double qvalue = qmap_iter->second;

         max_qmap_iter = max_qmap.find(turtle_cell);
         if(max_qmap_iter == max_qmap.end())
         {
            pair<double, int> P;
            P.first = qvalue;
            P.second = turtle_direction;
            max_qmap[turtle_cell] = P;
         }
         else
         {

// As of 11/17/16, we experiment with randomly breaking ties between
// max qmap values:

            if(nearly_equal(max_qmap_iter->second.first, qvalue))
            {
               if(nrfunc::ran1() > 0.5)
               {
                  max_qmap_iter->second.first = qvalue;
                  max_qmap_iter->second.second = turtle_direction;
               }
            }
            else if(max_qmap_iter->second.first < qvalue)
            {
               max_qmap_iter->second.first = qvalue;
               max_qmap_iter->second.second = turtle_direction;
            }
         }
         max_qmap_iter = max_qmap.find(turtle_cell);
      } // loop over index a labeling actions
   } // loop over index s labeling current maze states
}

// ---------------------------------------------------------------------
// Member function score_max_Qmap() loops over all entries within
// member STL map max_Qmap.  For each turtle cell, it compares the
// direction vector currently stored in max_Qmap with the solution
// direction stored in *soln_grid_ptr.  This method counts the number
// cells for which these two directions match.  It returns the ratio
// of matching directions to the number of cells as a metric of the
// Qmap's utility.

double maze::score_max_Qmap()
{
//   cout << "inside maze::score_max_Qmap()" << endl;
//   cout << "max_qmap.size = " << max_qmap.size() << endl;
   
   int n_correct_dirs = 0;
   for(max_qmap_iter = max_qmap.begin(); 
       max_qmap_iter != max_qmap.end(); max_qmap_iter++)
   {
      int turtle_cell = max_qmap_iter->first;
      int tx, ty;
      decompose_turtle_cell(turtle_cell, tx, ty);
      int col = tx / 2;
      int row = ty / 2;
      int max_qmap_direction = max_qmap_iter->second.second;
      int soln_direction = soln_grid_ptr->get(row,col);
//      cout << "row = " << row << " col = " << col
//           << " soln_dir = " << soln_direction
//           << " max_qmap_direction = " << max_qmap_direction << endl;
      if( (col == n_size - 1 && row == n_size - 1) ||
          (max_qmap_direction == soln_direction) ) n_correct_dirs++;
   }
   cout << "  n_correct_dirs = " << n_correct_dirs 
        << " max_qmap.size = " << max_qmap.size() << endl;
   double Qmap_score = double(n_correct_dirs) / n_cells;
   return Qmap_score;
}

// ---------------------------------------------------------------------
// Member function identify_max_Qmap_problems() searches for cells
// containing illegal actions and contradictory cell pairs.

void maze::identify_max_Qmap_problems()
{
   grid_problems_ptr->clear_values();
   current_problem_cells.clear();
   problem_cells_map.clear();

   for(max_qmap_iter = max_qmap.begin(); 
       max_qmap_iter != max_qmap.end(); max_qmap_iter++)
   {
      int turtle_cell = max_qmap_iter->first;
      int tx, ty;
      decompose_turtle_cell(turtle_cell, tx, ty);
      int px = tx / 2;
      int py = ty / 2;
      int p = get_cell(px, py);
      if(p == n_cells - 1) continue;
      int turtle_direction = max_qmap_iter->second.second;

      if(!legal_turtle_move(tx, ty, turtle_direction))
      {
         grid_problems_ptr->put(px, py, -2);
         
         current_problem_cells.push_back(p);
         current_problem_cells.push_back(p);
         problem_cells_map[turtle_cell] = -2;
         continue;
      }
      
      int p_neighbor = get_neighbor(p, turtle_direction);
      int px_neighbor = cell_decomposition[p_neighbor].first;
      int py_neighbor = cell_decomposition[p_neighbor].second;
      int tx_neighbor = 2 * px_neighbor;
      int ty_neighbor = 2 * py_neighbor;
      int turtle_cell_neighbor = get_turtle_cell(tx_neighbor, ty_neighbor);
      if(p_neighbor == n_cells - 1) continue;

      MAX_Q_MAP::iterator max_qmap_neighbor_iter = max_qmap.find(
         turtle_cell_neighbor);
      if(max_qmap_neighbor_iter == max_qmap.end())
      {
         cout << "Trouble in maze::locate_max_Qmap_problems()" << endl;
      }
      else
      {
         int neighbor_turtle_direction = max_qmap_neighbor_iter->second.second;
         if( (turtle_direction == 0 && neighbor_turtle_direction == 2) ||
             (turtle_direction == 1 && neighbor_turtle_direction == 3) ||
             (turtle_direction == 2 && neighbor_turtle_direction == 0) ||
             (turtle_direction == 3 && neighbor_turtle_direction == 1))
         {
            grid_problems_ptr->put(px, py, -1);
            current_problem_cells.push_back(p);
            current_problem_cells.push_back(p_neighbor);
            problem_cells_map[turtle_cell] = -1;
            problem_cells_map[turtle_cell_neighbor] = -1;
         }
      }
   } // loop over max_qmap_iter
}

// ---------------------------------------------------------------------
// Member function draw_max_Qmap() draws an arrow in each cell which
// indicates the maximal Q value for that position.

void maze::draw_max_Qmap(unsigned char* img)
{
   identify_max_Qmap_problems();
   
   for(max_qmap_iter = max_qmap.begin(); 
       max_qmap_iter != max_qmap.end(); max_qmap_iter++)
   {

      int turtle_cell = max_qmap_iter->first;
      int tx, ty;
      decompose_turtle_cell(turtle_cell, tx, ty);
      int px = tx / 2;
      int py = ty / 2;
      int turtle_direction = max_qmap_iter->second.second;

      int R = 0;
      int G = 255;
      int B = 0;

      if(nearly_equal(grid_problems_ptr->get(px,py), -2))
      {
         R = 255;
         G = 0;
         B = 0;
      }

// Color contradictory neighbor pair directions orange:
      else if(nearly_equal(grid_problems_ptr->get(px,py), -1))
      {
         R = 255;
         G = 123;
         B = 0;
      }

// Draw bluish "X" within final cell location to indicate it is the
// maze's termination point:

      if(px == n_size - 1 && py == n_size - 1)
      {
         R = 0;
         G = 128;
         B = 255;
         DrawCellX(img, px, py, R, G, B);
      }
      else
      {
         DrawCellArrow(img, px, py, turtle_direction, R, G, B);
      }
   } // loop over max_qmap_iter
}

// ---------------------------------------------------------------------
// Member function color_cells() fills each maze cell with a color
// which is correlated to an underlying trained weight value.

void maze::color_cells(unsigned char* img, twoDarray* wtwoDarray_ptr)
{
   cout << "inside maze::color_cells, img = " << img << endl;
   cout << "max_qmap.size = " << max_qmap.size() << endl;
   cout << "*wtwoDArray_ptr = " << *wtwoDarray_ptr << endl;
   
   for(max_qmap_iter = max_qmap.begin(); 
       max_qmap_iter != max_qmap.end(); max_qmap_iter++)
   {
      int turtle_cell = max_qmap_iter->first;
      int tx, ty;
      decompose_turtle_cell(turtle_cell, tx, ty);
      cout << "turtle_cell = " << turtle_cell 
           << " tx = " << tx << " ty = " << ty << endl;
      int px = tx / 2;
      int py = ty / 2;

      double curr_w = wtwoDarray_ptr->get(tx,ty);
      cout << "curr_w = " << curr_w << endl;
      double h = (1 - curr_w/255.0) * 270;
      double s = 1;
      double v = 0.4;
      double r, g, b;
      colorfunc::hsv_to_RGB(h, s, v, r, g, b);
      int R = 255 * r;
      int G = 255 * g;
      int B = 255 * b;
      cout << "px = " << px << " py = " << py << " R = " << G
           << " G = " << G << " B = " << B << endl;
      FillCell(img, px, py, R, G, B);
      cout << "After FillCell" << endl;
   } // loop over max_qmap_iter
   cout << "at end of maze::color_cells" << endl;
}

// ---------------------------------------------------------------------
// Fill (2*n_size - 1) x (2*n_size - 1) occupancy_grid with 0s
// indicating vacant cells and 1s indicating wall cells.

void maze::initialize_occupancy_grid()
{
   int mdim = occupancy_grid->get_mdim();
   int ndim = occupancy_grid->get_ndim();
   
   for(int py = 0; py < n_size; py++)
   {
      for(int px = 0; px < n_size; px++)
      {
         string cell_bitstr = get_cell_bitstr(px, py);
//         cout << "px = " << px << " py = " << py 
//              << " bitstr = " << cell_bitstr << endl;

         int qx = 2 * px;
         int qy = 2 * py;
         occupancy_grid->put(qx, qy, 0);
         
         if(qx+1 < ndim)
         {
            if(cell_bitstr.substr(2,1) == "1") // wall to right of (px,py)
            {
               occupancy_grid->put(qx+1, qy, wall_value);
            }
            else
            {
               occupancy_grid->put(qx+1, qy, 0);
            }
         }
         
         if(qy+1 < mdim)
         {
            if(cell_bitstr.substr(1,1) == "1") // wall below (px,py)
            {
               occupancy_grid->put(qx, qy+1, wall_value);
            }
            else
            {
               occupancy_grid->put(qx, qy+1, 0);
            }
         }
         
         if(qx+1 < ndim && qy+1 < mdim)
         {
            occupancy_grid->put(qx+1, qy+1, wall_value);
         }

      } // loop over px
   }  // loop over py

   initialize_occupancy_state();
}

// ---------------------------------------------------------------------
void maze::initialize_occupancy_state()
{
   int n_wall_cells = 0;
   for(unsigned int py = 0; py < occupancy_grid->get_mdim(); py++)
   {
      for(unsigned int px = 0; px < occupancy_grid->get_ndim(); px++)
      {
         if(nearly_equal(occupancy_grid->get(px,py), wall_value))
         {
            n_wall_cells++;
         }
      }
   }

// Renormalize wall_value so that sum over all occupancy_grid values =
// 0:

   double renorm_wall_value = -turtle_value / n_wall_cells;

   int cell = 0;   
   for(unsigned int py = 0; py < occupancy_grid->get_mdim(); py++)
   {
      for(unsigned int px = 0; px < occupancy_grid->get_ndim(); px++)
      {
         if(nearly_equal(occupancy_grid->get(px,py), wall_value))
         {
            occupancy_grid->put(px,py,renorm_wall_value);
         }
         occupancy_state->put(cell, occupancy_grid->get(px,py));
         cell++;
      }
   }

   wall_value = renorm_wall_value;
}

// ---------------------------------------------------------------------
void maze::print_occupancy_grid() const
{
   cout << "occupancy_grid: mdim = "  
        << occupancy_grid->get_mdim()
        << " ndim = " << occupancy_grid->get_ndim() << endl;
   for(unsigned int py = 0; py < occupancy_grid->get_mdim(); py++)
   {
      for(unsigned int px = 0; px < occupancy_grid->get_ndim(); px++)
      {
//         if(nearly_equal(occupancy_grid->get(px,py), turtle_value))
//         {
//            cout << "T" << flush;
//         }
//         else
         {
            double curr_occup_val = occupancy_grid->get(px,py);
            int prec = 3;
            if(curr_occup_val < 0)
            {
               prec = 2;
            }
            cout << stringfunc::number_to_string(curr_occup_val,prec) 
                 << " " << flush;
         }
      }
      cout << endl;
   }
   cout << endl;
}

// ---------------------------------------------------------------------
void maze::print_occupancy_state() const
{
   for(unsigned int p = 0; p < occupancy_state->get_mdim(); p++)
   {
      cout << occupancy_state->get(p) << " " << flush;
   }
   cout << endl;
}

// ---------------------------------------------------------------------
// Member function occupancy_state_to_string() converts genvector
// *occupancy_state into a corresponding string containing "T" for
// turtle's location, "W" for wall locations and "E" for empty cell
// locations.  The string can be used as a key inside STL maps.

string maze::occupancy_state_to_string() 
{
   string occup_state_str="";
   for(unsigned int p = 0; p < occupancy_state->get_mdim(); p++)
   {
      double cell_value = occupancy_state->get(p);
      if(cell_value > 0)
      {
         occup_state_str += "T";
      }
      else if(cell_value < 0)
      {
         occup_state_str += "W";
      }
      else
      {
         occup_state_str += "E";
      }
   }
   return occup_state_str;
}

// ---------------------------------------------------------------------
// Member function generate_all_turtle_states() works with the current
// maze which is assumed to contain no turtle.  It moves the turtle
// into all possible n_size x n_size locations and saves the
// corresponding state vector as a string into member STL vector
// curr_maze_state_strings.

void maze::generate_all_turtle_states()
{
   curr_maze_states.clear();
   curr_maze_state_strings.clear();
   initialize_occupancy_grid();

   for(int py = 0; py < n_size; py++)
   {
      int ty = 2 * py;
      for(int px = 0; px < n_size; px++)
      {
         int tx = 2 * px;
         int turtle_cell = get_turtle_cell(tx, ty);
         occupancy_state->put(turtle_cell, turtle_value);

         genvector* turtle_state = new genvector(*occupancy_state);
         curr_maze_states.push_back(turtle_state);

         string state_str =  occupancy_state_to_string();    
         curr_maze_state_strings.push_back(state_str);

         occupancy_state->put(turtle_cell, 0);
      } // loop over px
   } // loop over py
}

// ---------------------------------------------------------------------
void maze::reset_game(bool random_turtle_start)
{
   game_over = false;
   this->random_turtle_start = random_turtle_start;

   initialize_occupancy_grid();

   turtle_cell = 0;
   int tx = 0;
   int ty = 0;
   if(random_turtle_start)
   {

/*
// Experiments with systematically starting turtle in each maze cell
// yields no obvious benefit compared to randomly starting turtle
// anywhere within maze

         turtle_px_start = cell_decomposition[turtle_p_start].first;
         turtle_py_start = cell_decomposition[turtle_p_start].second;
         turtle_p_start++;
         if(turtle_p_start >= n_cells)
         {
            turtle_p_start = 0;
         }
*/
       
      turtle_px_start = mathfunc::getRandomInteger(n_size);
      turtle_py_start = mathfunc::getRandomInteger(n_size);

/*
      int n_curr_problem_cells = current_problem_cells.size();
      if(n_curr_problem_cells > 0 &&
         n_curr_problem_cells < 0.5 * n_cells)
      {
         bool OK_turtle_starting_location = false;
         while(!OK_turtle_starting_location)
         {
            turtle_px_start = mathfunc::getRandomInteger(n_size);
            turtle_py_start = mathfunc::getRandomInteger(n_size);
            int turtle_p_start = get_turtle_cell(
               turtle_px_start, turtle_py_start);
            OK_turtle_starting_location = true;
            for(int i = 0; i < n_curr_problem_cells; i++)
            {
               if(turtle_p_start == current_problem_cells[i])
               {
                  OK_turtle_starting_location = false;
               }
            }
         }
      }
      else
      {
         turtle_px_start = mathfunc::getRandomInteger(n_size);
         turtle_py_start = mathfunc::getRandomInteger(n_size);
      }
*/
      
      tx = 2 * turtle_px_start;
      ty = 2 * turtle_py_start;     
      turtle_cell = get_turtle_cell(tx, ty);
   }
   occupancy_grid->put(tx, ty, turtle_value);   
   occupancy_state->put(turtle_cell, turtle_value);

   visited_turtle_cells->clear_values();
   visited_turtle_cells->put(tx, ty, 1);

   turtle_path_history.clear();
   turtle_path_history.push_back(0);
}

// ---------------------------------------------------------------------
void maze::set_game_over(bool flag)
{
   game_over = true;
}

// ---------------------------------------------------------------------
bool maze::get_game_over() const
{
   return game_over;
}

// ---------------------------------------------------------------------
bool maze::get_maze_solved() const
{
   return (turtle_cell == int(occupancy_state->get_mdim() - 1));
}

// ---------------------------------------------------------------------
int maze::get_n_soln_steps() const
{
   return soln_path.size();
}

// ---------------------------------------------------------------------
// Member function compute_legal_turtle_actions() fills genvector
// *curr_legal_actions with -1 entries for actions which are ILLEGAL
// and 0 entries for actions which are LEGAL.

genvector* maze::compute_legal_turtle_actions()
{
   curr_legal_actions->initialize_values(-1);
   for(int curr_dir = 0; curr_dir < n_directions; curr_dir++)
   {
      if(legal_turtle_move(curr_dir))
      {
         curr_legal_actions->put(curr_dir, 0);
      }
   }
   return curr_legal_actions;
}

// ---------------------------------------------------------------------
bool maze::legal_turtle_move(int curr_dir)
{
   int tx = occupancy_cell_decomposition[turtle_cell].first;
   int ty = occupancy_cell_decomposition[turtle_cell].second;
   return legal_turtle_move(tx, ty, curr_dir);
}

// ---------------------------------------------------------------------
bool maze::legal_turtle_move(int tx, int ty, int curr_dir)
{
   DUPLE d = getDirection(curr_dir);
   int tx_new = tx + 2 * d.first;
   int ty_new = ty + 2 * d.second;

   if(tx_new < 0 || tx_new >= int(occupancy_grid->get_ndim()) ||
      ty_new < 0 || ty_new >= int(occupancy_grid->get_mdim()) )
   {
      return false;        // Turtle cannot move beyond grid borders
   }

   int tx_inter = tx + d.first;
   int ty_inter = ty + d.second;

   int t_inter = get_occupancy_cell(tx_inter,ty_inter);
   if(fabs(occupancy_state->get(t_inter)) > 0)
   {
      return false;	 // new cell is already occupied by wall
   }

/*
// Fri Nov 18 at 6:56 am

// Experiment with forbidding turtle to ever move to a previously
// visited cell:

   int max_prev_visits = 4 * n_size;
//   cout << "tx = " << tx_new << " ty = " << ty_new 
//        << " n_prev_visits = " << visited_turtle_cells->get(tx_new, ty_new)
//        << endl;
   if(visited_turtle_cells->get(tx_new,ty_new) > max_prev_visits)
   {
      return false;
   }
*/

   return true;
}

// ---------------------------------------------------------------------
int maze::move_turtle(int curr_dir, bool erase_turtle_path)
{
//   cout << "inside move_turtle, curr_dir = " << curr_dir << endl;
   int tx = occupancy_cell_decomposition[turtle_cell].first;
   int ty = occupancy_cell_decomposition[turtle_cell].second;
//   cout << "Before move, tx = " << tx << " ty = " << ty << endl;

   DUPLE d = getDirection(curr_dir);
   int tx_inter = tx + d.first;
   int ty_inter = ty + d.second;
   int tx_new = tx + 2 * d.first;
   int ty_new = ty + 2 * d.second;
//   cout << "direction : dx = " << d.first << " dy = " << d.second << endl;
//   cout << "After move, tx_new = " << tx_new <<" ty = " << ty_new << endl;

   if(tx_new < 0 || tx_new >= int(occupancy_grid->get_ndim()) ||
      ty_new < 0 || ty_new >= int(occupancy_grid->get_mdim()) )
   {
      return -1;        // Turtle cannot move beyond grid borders
   }
   
   int t_inter = get_occupancy_cell(tx_inter,ty_inter);
//   cout << "t_new = " << t_new 
//        << " occupancy_state(t_new) = " << occupancy_state->get(t_new)
//        << endl;

   if(fabs(occupancy_state->get(t_inter)) > 0)
   {
      return -1;	 // new cell is already occupied by wall
   }

   occupancy_state->put(turtle_cell, 0);

   int t_new = get_occupancy_cell(tx_new,ty_new);
   turtle_cell = t_new;
//   cout << "turtle_cell = " << turtle_cell << endl;
   occupancy_state->put(turtle_cell, turtle_value);

   if(erase_turtle_path)
   {
      occupancy_grid->put(tx,ty,0);
   }
   occupancy_grid->put(tx_new, ty_new, turtle_value);
   visited_turtle_cells->put(
      tx_new, ty_new, visited_turtle_cells->get(tx_new, ty_new) + 1);

   if (turtle_cell == int(occupancy_state->get_mdim() - 1))
   {
      game_over = true;
   }

   turtle_path_history.push_back(t_new);
   return turtle_cell;
}

// ---------------------------------------------------------------------
void maze::print_turtle_path_history() const
{
   for(unsigned int i = 0; i < turtle_path_history.size(); i++)
   {
      cout << "i = " << i << " turtle cell = " << turtle_path_history[i]
           << endl;
   }
   cout << "---------------" << endl;
}

// ---------------------------------------------------------------------
int maze::get_n_turtle_moves() const
{
   return turtle_path_history.size();
}

// ---------------------------------------------------------------------
int maze::compute_turtle_reward() const
{
   return (get_maze_solved());

/*
   if(get_maze_solved())
   {
      if(random_turtle_start)
      {
         int approx_reward = abs(n_size - turtle_px_start) + 
            abs(n_size - turtle_py_start);
         return approx_reward;
      }
      else
      {
         return 1;
      }
   }
   else
   {
      return 0;
   }
*/

  /*
   if(get_n_turtle_moves() < get_solution_path_moves())
   {
      return 0;
   }
   else if (!get_maze_solved())
   {
      return 0;
   }
   else
   {
      int reward = 1 - ( get_n_turtle_moves() - get_solution_path_moves() );
      if(reward > 1) reward = 0;
      return reward;
   }
  */
}

// ---------------------------------------------------------------------
// Member function solve_maze_backwards() initializes the current maze
// cell to the maze's terminal location in the bottom right corner.  It
// recursively finds directions pointing from neighboring cells to the
// current cell.

void maze::solve_maze_backwards()
{
//   cout << "inside solve_maze_backwards()" << endl;

   soln_grid_ptr->initialize_values(-2);

   int px = n_size - 1;
   int py = n_size - 1;
   int p = get_cell(px, py);
   soln_grid_ptr->put(px,py,-1);
   
   find_neighbor_cell_directions(p);
}

// ---------------------------------------------------------------------
// Member function find_neighbor_cell_directions() takes in index p 
// for some maze cell.  Looping over up, right, down and left
// directions, it ignores any direction for which the input cell has a
// wall.  It also ignores any direction for which the neighbor cell n
// already has an assigned direction.  Otherwise, neighbor cell n is
// assigned the direction pointing towards cell p.  The method is then
// recursively called on neighbor cell n.

void maze::find_neighbor_cell_directions(int p)
{
//   cout << "inside find_neighbor_cell_directions, p = " << p << endl;
   for(int curr_dir = 0; curr_dir < n_directions; curr_dir++)
   {
      if(wall_exists(p, curr_dir)) continue;
      int n = get_neighbor(p, curr_dir);
//      cout << "n = " << n << endl;
      int col = cell_decomposition[n].first;
      int row = cell_decomposition[n].second;
      if(soln_grid_ptr->get(row, col) >= -1) continue;

      int anti_curr_dir = (curr_dir + 2) % 4;
      soln_grid_ptr->put(row, col, anti_curr_dir);
//      cout << "row = " << row << " col = " << col << " anti_curr_dir = "
//           << anti_curr_dir << endl;
      find_neighbor_cell_directions(n);
   } // loop over curr_dir
   return;
}

// ---------------------------------------------------------------------
void maze::print_soln_grid() const
{
   cout << "soln_grid = " << *soln_grid_ptr << endl;
}

// ---------------------------------------------------------------------
// Member function is_problem_cell()

int maze::is_problem_cell(int turtle_p)
{
   problem_cells_iter = problem_cells_map.find(turtle_p);
   if(problem_cells_iter == problem_cells_map.end()) 
   {
      return 0;
   }
   return problem_cells_iter->second;
}
