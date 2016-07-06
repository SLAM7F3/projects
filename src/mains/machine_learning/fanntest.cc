#include <iostream>
#include <floatfann.h>

using std::cin;
using std::cout;
using std::endl;

int main()
{
   fann_type *calc_out;
   fann_type input[2];
   struct fann *ann = fann_create_from_file("xor_float.net");

   fann_type input_1,input_2;
   cout << "Enter input 1:" << endl;
   cin >> input_1;

   cout << "Enter input 2:" << endl;
   cin >> input_2;
  
   input[0]=input_1;
   input[1]=input_2;
 
//   input[0] = -1;
//   input[1] = 1;
   calc_out = fann_run(ann, input);

   cout << "Xor results = " << calc_out[0] << endl;
//   printf("xor test (%f,%f) -> %f\n", input[0], input[1], calc_out[0]);
   fann_destroy(ann);
   return 0;
}

