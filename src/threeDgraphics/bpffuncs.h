// =========================================================================
// Header file for stand-alone BPF data manipulation functions.
// =========================================================================
// Last modified on 11/16/11; 11/18/11; 11/28/11
// =========================================================================

#ifndef BPFFUNCS_H
#define BPFFUNCS_H

#include <iostream>
#include <string>
#include <vector>

namespace bpffunc
{
   bool is_bpf3_file(std::ifstream& binary_instream);

   void parse_bpf12_points(
      std::ifstream& binary_instream,
      std::vector<double>* X_ptr,std::vector<double>* Y_ptr,
      std::vector<double>* Z_ptr,std::vector<double>* NWC_ptr,
      std::vector<double>* P_ptr,int& UTM_zonenumber);
   int parse_L1_bpf3_points(
      std::ifstream& binary_instream,
      std::vector<double>* X_ptr,std::vector<double>* Y_ptr,
      std::vector<double>* Z_ptr,std::vector<int>* pixel_number_ptr,
      int& UTM_zonenumber);
   void parse_bpf3_points(
      std::ifstream& binary_instream,
      std::vector<double>* X_ptr,std::vector<double>* Y_ptr,
      std::vector<double>* Z_ptr,std::vector<double>* NWC_ptr,
      std::vector<double>* P_ptr,std::vector<int>* pixel_number_ptr,
      int& UTM_zonenumber,int& n_frames);

   int read_bpf_L1_data(
      std::string bpf_filename,std::vector<double>* X_ptr,
      std::vector<double>* Y_ptr,std::vector<double>* Z_ptr,
      std::vector<int>* pixel_number_ptr,int& UTM_zonenumber);
}

#endif // bpffuncs.h




