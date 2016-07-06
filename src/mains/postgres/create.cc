// ========================================================================
// Program PQTEST 
// ========================================================================
// Last updated on 9/28/06
// ========================================================================

#include <iostream>
#include <vector>
#include <libpq-fe.h>
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "postgres/database.h"

int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::ifstream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

   string hostname="localhost";
   string database_name="bpsimple";
   string username="cho";
   database db(hostname,database_name,username);

   db.connect();

   vector<string> commands;
   commands.push_back("drop table number");
   commands.push_back("create table number (value integer, name varchar)");
   commands.push_back("insert into number values(42, 'The answer')");
   commands.push_back("insert into number values(29, 'My age')");
   commands.push_back("insert into number values(29, 'Anniversary')");
   commands.push_back("insert into number values(66, 'clickety-click')");

/*
   for (int n=0; n<10; n++)
   {
      string curr_command="insert into number values("+
         stringfunc::number_to_string(n)+" ,'single digit')";
      commands.push_back(curr_command);
   }
*/

//   commands.push_back("update number set name='Zaphod' where value = 42");
//   commands.push_back("delete from number where value = 29");

   commands.push_back("select * from number where value=29");


   db.set_SQL_commands(commands);
   db.execute_SQL_commands();

   db.disconnect();

}

