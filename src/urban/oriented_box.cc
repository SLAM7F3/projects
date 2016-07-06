// ==========================================================================
// oriented_box class member function definitions
// ==========================================================================
// Last modified on 4/22/06; 5/22/06; 6/10/06; 4/5/14
// ==========================================================================

#include "urban/oriented_box.h"
#include "urban/rooftop.h"

using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::vector;

// ==========================================================================
// Initialization, constructor and destructor methods:
// ==========================================================================

void oriented_box::allocate_member_objects()
{
   roof_ptr=new rooftop();
   face_label=new sideface_type[n_sidefaces];
}		       

void oriented_box::initialize_member_objects()
{
   for (unsigned int n=0; n<n_sidefaces; n++)
   {
      face_label[n]=back;
      side_face[n].vertex_average();
   }
   calculate_symmetry_vectors_and_lengths();
}

oriented_box::oriented_box(void)
{
   allocate_member_objects();
}

oriented_box::oriented_box(double w,double l,double h):
   mybox(w,l,h)
{
   allocate_member_objects();
   initialize_member_objects();
}

oriented_box::oriented_box(
   double xlo,double xhi,double ylo,double yhi,double zlo,double zhi):
   mybox(xlo,xhi,ylo,yhi,zlo,zhi)
{
   allocate_member_objects();
   initialize_member_objects();
}

oriented_box::oriented_box(const polygon& bface,const threevector& zhat,
                           double h):mybox(bface,zhat,h)
{
   allocate_member_objects();
   initialize_member_objects();
}

// Copy constructor:

oriented_box::oriented_box(const oriented_box& o):
   mybox(o)
{
   allocate_member_objects();
   docopy(o);
}

oriented_box::~oriented_box()
{
//   cout << "inside oriented_box destructor" << endl;
   delete roof_ptr;
//   cout << "Before call to delete [] face_label in oriented_box destructor"
//        << endl;
   delete [] face_label;
   roof_ptr=NULL;
   face_label=NULL;
//   cout << "At end of oriented box destructor" << endl;
}

// ---------------------------------------------------------------------
void oriented_box::docopy(const oriented_box& o)
{
   *roof_ptr=*(o.roof_ptr);
   for (unsigned int n=0; n<n_sidefaces; n++)
   {
      face_label[n]=o.face_label[n];
   }
}	

// Overload = operator:

oriented_box oriented_box::operator= (const oriented_box& o)
{
   if (this==&o) return *this;
   mybox::operator=(o);
   docopy(o);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,oriented_box& o)
{

/*
   outstream << endl;
   for (unsigned int i=0; i<o.n_sidefaces; i++)
   {
      outstream << "box side face " << i << " : orientation label = ";
      if (o.face_label[i]==oriented_box::front) 
      {
         outstream << "front" << endl;
      }
      else if (o.face_label[i]==oriented_box::back) 
      {
         outstream << "back" << endl;
      }
      else
      {
         outstream << "side" << endl;
      }
   }
*/

   outstream << (parallelepiped&)o << endl;
   o.print_rooftop_information();
//   if (o.roof_ptr != NULL) outstream << *(o.roof_ptr) << endl;
   return(outstream);
}

// ---------------------------------------------------------------------
void oriented_box::print_rooftop_information()
{
   if (roof_ptr==NULL)
   {
      cout << "No rooftop information computed for current box" << endl;
   }
   else
   {
      if (roof_ptr->get_none_flag())
      {
         cout << "Box has roof of unknown shape" << endl;
      }
      else
      {
         if (roof_ptr->get_flat_flag())
         {
            cout << "Box has flat roof" << endl;
         }
         else if (roof_ptr->get_pyramid_flag())
         {
            cout << "Box has pyramid roof" << endl;
            threevector pyramid_apex(roof_ptr->get_COM());
            cout << "Pyramid apex = " << pyramid_apex << endl;
         }
         else if (roof_ptr->get_spine_ptr() != NULL)
         {
            cout << "Box has spined roof" << endl;
            cout << "Spine = " << 
               *(roof_ptr->get_spine_ptr()) << endl;
            
            linesegment const *rooftop_spine_ptr=roof_ptr->get_spine_ptr();
            for (unsigned int i=0; i<top_face.get_nvertices(); i++)
            {
               threevector curr_vertex(top_face.get_vertex(i));
               threevector v1(rooftop_spine_ptr->get_v1());
               threevector v2(rooftop_spine_ptr->get_v2());
               double dist1=(curr_vertex-v1).magnitude();
               double dist2=(curr_vertex-v2).magnitude();
               if (dist1 < dist2)
               {
                  cout << "Line connecting top face to spine: " 
                       << linesegment(curr_vertex,v1) << endl;
               }
               else
               {
                  cout << "Line connecting top face to spine: " 
                       << linesegment(curr_vertex,v2) << endl;
               }
            } // loop over index i labeling top face vertex
         } // rooftop_spine_ptr != NULL conditional
      } // !none_flag conditional
   } // roof_ptr==NULL conditional
}

// =========================================================================
// Text file I/O
// ==========================================================================

ostream& oriented_box::write_to_textstream(ostream& textstream)
{
   parallelepiped::write_to_textstream(textstream);
   if (roof_ptr != NULL)
   {
      roof_ptr->write_to_textstream(textstream);
   }
   else
   {
      textstream << "NULL" << endl;
   }
   return textstream;
}

// ---------------------------------------------------------------------
void oriented_box::read_from_text_lines(unsigned int& i,vector<string>& line)
{
   parallelepiped::read_from_text_lines(i,line);
   roof_ptr->read_from_text_lines(i,line);
//   cout << "*this = " << *this << endl;
}
