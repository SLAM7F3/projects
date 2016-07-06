/* This is free and unencumbered software released into the public domain. */

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <lmdb++.h>

using std::string;
using std::vector;


int main() 
{
   string input_subdir = "./examples/_temp/features_animals/";
//   string input_subdir = "./example.mdb";

   // Create and open the LMDB environment:


   auto env = lmdb::env::create();
   env.set_mapsize(1UL * 1024UL * 1024UL * 1024UL); // 1 GiB 
   env.open(input_subdir.c_str(), 0, 0664);
   
   // Insert some key/value pairs in a write transaction: 
   auto wtxn = lmdb::txn::begin(env);
   auto dbi = lmdb::dbi::open(wtxn, nullptr);

/*
   dbi.put(wtxn, "username", "jhacker");
   dbi.put(wtxn, "email", "jhacker@example.org");
   dbi.put(wtxn, "fullname", "J. Random Hacker");

*/
   wtxn.commit();


   // Fetch key/value pairs in a read-only transaction: 
   auto rtxn = lmdb::txn::begin(env, nullptr, MDB_RDONLY);
   auto cursor = lmdb::cursor::open(rtxn, dbi);
   std::string key, value;
   while (cursor.get(key, value, MDB_NEXT)) {
      std::printf("key: '%s', value: '%s'\n", key.c_str(), value.c_str());
   }
   cursor.close();
   rtxn.abort();

   // The enviroment is closed automatically.

   return EXIT_SUCCESS;
}
