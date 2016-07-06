// ==========================================================================
// Program HASHTEST
// ==========================================================================
// Last updated on 3/9/04
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include "general/sysfuncs.h"
#include "genfuncs.h"
#include "general/outputfuncs.h"
#include "datastructures/Hashtable.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::string;
   using std::ostream;
   using std::istream;
   using std::ifstream;
   using std::ofstream;
   using std::cout;
   using std::cin;
   using std::ios;
   using std::endl;
   std::set_new_handler(sysfunc::out_of_memory);
  
   int n_objects=4;
   Hashtable<int>* hashtable_ptr=new Hashtable<int>(2*n_objects);

   for (int n=0; n<n_objects; n++)
   {
      int key=n;
      int curr_object=sqr(n);
      
      Mynode<int>* keynode_ptr=
         hashtable_ptr->insert_key(key,curr_object);
   }
   cout << "hashtable = " << *hashtable_ptr << endl;

//   Hashtable<int>* new_hashtable_ptr=new Hashtable<int>(2*n_objects);
//   *new_hashtable_ptr=*hashtable_ptr;
//   cout << "new_hashtable = " << *new_hashtable_ptr << endl;

   Hashtable<int>* new_hashtable_ptr=new Hashtable<int>(*hashtable_ptr);
   cout << "new_hashtable = " << *new_hashtable_ptr << endl;

   exit(-1);

   while (true)
   {
      int n;
      cout << "Enter integer n:" << endl;
      cin >> n;
      if (hashtable_ptr->data_in_hashtable(n) != NULL)
      {
         cout << "n is in the hashtable" << endl;
      }
      else
      {
         cout << "n is NOT in the hashtable" << endl;
      }
      outputfunc::newline();
   }
   

}


