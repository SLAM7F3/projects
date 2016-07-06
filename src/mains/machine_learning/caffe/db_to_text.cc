// Copyright 2014 BVLC and contributors.

#include <glog/logging.h>
#include <stdio.h>  // for snprintf
#include <google/protobuf/text_format.h>
#include <leveldb/db.h>
#include <leveldb/write_batch.h>
#include <string>
#include <vector>
#include <cassert>
#include <iostream>
#include <map>

#include "caffe/proto/caffe.pb.h"
#define NUMBER_FEATURES_PER_IMAGE 16
using namespace std;
using namespace caffe;

int main(int argc, char** argv)
{
   //google::InitGoogleLogging(argv[0]);
   if (argc < 2)
   {
      printf("ERROR! Not enough arguments!\nusage: %s <feature_folder>", 
             argv[0]);
      exit(1);
   }

   LOG(INFO) << "Creating leveldb object\n";
   leveldb::DB* db;
   leveldb::Options options;
   options.create_if_missing = true;
   leveldb::Status status = leveldb::DB::Open(options, argv[1], &db);
   assert(status.ok());

   leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions());
   int i = 0;
   double count = 0.0f;
   for (it->SeekToFirst(); it->Valid(); it->Next()) 
   {
      Datum d;
      d.clear_float_data();
      d.clear_data();
      d.ParseFromString(it->value().ToString()); 
      for (int j = 0; j < d.height(); ++j)
         count += d.float_data(j);
      i++;
   }
   assert(it->status().ok());

   LOG(INFO) << "Number of datums (or feature vectors): " << i << "\n";;
   LOG(INFO) << "Reduction of All Vectors to A Scalar Value: " << count << "\n";
   delete it;
}

