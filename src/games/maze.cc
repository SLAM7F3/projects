// ==========================================================================
// maze class member function definitions
// ==========================================================================
// Last modified on 11/5/16
// ==========================================================================

#include <iostream>
#include <string>
#include "math/mathfuncs.h"
#include "games/maze.h"
#include "numrec/nrfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

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
   ImageSize = 512;
}		       

void maze::initialize_member_objects()
{
   game_over = false;

   direction.push_back(1); // up
   direction.push_back(2); // right
   direction.push_back(4); // down
   direction.push_back(8); // left

// Load cell_decomposition vector with (px,py,pz) triples
// corresponding to p = 0 --> n_cells - 1:

   for(int q = 0; q < n_cells; q++)
   {
      int p = q;
      int py = p / n_size;
      p -= n_size * py;
      int px = p;
      cell_decomposition.push_back(DUPLE(px,py));
   }

// Set all visited cell flags to false:

   for(int p = 0; p < n_cells; p++)
   {
      visited_cell.push_back(false);
   }
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
   generate_maze();
}

// ---------------------------------------------------------------------
maze::~maze()
{
   delete grid_ptr;
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
   if(curr_dir == 0)
   {
      return DUPLE(0,-1);
   }
   else if(curr_dir == 1)
   {
      return DUPLE(1,0);
   }
   else if(curr_dir == 2)
   {
      return DUPLE(0,1);
   }
   else if(curr_dir == 3)
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
void maze::remove_wall(int p, int curr_dir)
{
//   cout << "inside remove_wall, p = " << p << " curr_dir = " << curr_dir
//        << endl;
   
   int px = cell_decomposition[p].first;
   int py = cell_decomposition[p].second;
//   cout << "px = " << px << " py = " << py << endl;

   string cell_bitstr = get_cell_bitstr(px, py);

// Reset bit labeled by curr_dir to zero:

   cell_bitstr.replace(nbits-1-curr_dir, 1, "0");
   double new_cell_value = stringfunc::bits_rep_to_integer(cell_bitstr);
   grid_ptr->put(px, py, new_cell_value);
}

// ---------------------------------------------------------------------
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
int maze::get_n_unvisited_neighbors(int p)
{
   int n_unvisited_neighbors = 0;
   
   vector<int> cell_neighbor_ids = get_cell_neighbors(p);
   for(unsigned int n = 0; n < cell_neighbor_ids.size(); n++)
   {
      int cell_neighbor_id = cell_neighbor_ids[n];
      if(cell_neighbor_id < 0) continue;
      if(visited_cell[cell_neighbor_id] == false)
      {
         n_unvisited_neighbors++;
      }
   }
   return n_unvisited_neighbors;
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
void maze::generate_maze()
{
   cout << "inside generate_maze()" << endl;

   init_grid();
   print_grid();

// Start maze generation at some random location within grid:

   int p = mathfunc::getRandomInteger(n_cells);
   visited_cell[p] = true;
   visited_cell_stack.push(p);

   while(n_visited_cells() < n_cells)
   {
      while(get_n_unvisited_neighbors(p) > 0)
      {
         int curr_dir;
         int q = -1;
         while(q < 0)
         {
            curr_dir = mathfunc::getRandomInteger(4);
            q = get_neighbor(p, curr_dir);   
         }
      
         int px = cell_decomposition[p].first;
         int py = cell_decomposition[p].second;
         int qx = cell_decomposition[q].first;
         int qy = cell_decomposition[q].second;

         cout << "p = " << p 
              << " px = " << px << " py = " << py << endl;
         cout << "curr_dir = " << curr_dir << endl;
         cout << "qx = " << qx << " qy = " << qy << endl;

// Remove walls between (px,py) and (qx,qy):

         remove_wall(p, curr_dir);
         remove_wall(q, (curr_dir+2)%4);


         visited_cell[q] = true;
         visited_cell_stack.push(q);

         print_grid();
         DrawMaze();
         
         p = q;
      } // p has unvisited neighbors while loop
   
      cout << "Reached dead end at p = " << p << endl;
      cout << "visited_cell_stack.size() = " << visited_cell_stack.size()
           << endl;
      p = visited_cell_stack.top();
      visited_cell_stack.pop();
      cout << "Backtrack to p = " << p << endl;
      print_grid();

      DrawMaze();

   } // n_visited_cells < n_cells while loop
}

// ---------------------------------------------------------------------
// Bitmap note: x = y = 0 corresponds to BOTTOM left corner of output
// image rather than to TOP left corner!

void maze::DrawLine( unsigned char* img, int x1, int y1, int x2, int y2 ,
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

// ---------------------------------------------------------------------
void maze::RenderMaze( unsigned char* img )
{
   int CellSize = ImageSize / n_cells;
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
   cout << "Exported " << FileName << endl;
   string unix_cmd = "convert -flip "+FileName+" "+FileName;
   sysfunc::unix_command(unix_cmd);
}

// ---------------------------------------------------------------------
void maze::DrawMaze()
{
   // prepare BGR image
   size_t DataSize = 3 * ImageSize * ImageSize;

   unsigned char* Img = new unsigned char[ DataSize ];

   memset( Img, 0, DataSize );

   // render maze on bitmap
   RenderMaze( Img );

   SaveBMP( "Maze.bmp", Img, ImageSize, ImageSize );

   // cleanup
   delete[]( Img );

}
