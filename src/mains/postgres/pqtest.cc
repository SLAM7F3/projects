// ========================================================================
// Program PQTEST 
// ========================================================================
// Last updated on 9/28/06
// ========================================================================

#include <iostream>
#include <vector>
#include <libpq-fe.h>
#include "general/sysfuncs.h"
#include "postgres/database.h"

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::string;
using std::vector;

void insert_command(database* db_ptr)
{
   vector<string> commands;
   string insert_command="insert into customer(fname,lname,zipcode) ";
//   insert_command += "values('Peter','Cho','01821')";
//   insert_command += "values('Delsey','Sherrill','01821')";
//   insert_command += "values('Baby','Kermie','12345')";
   insert_command += "values('Beebop','One','45675')";
   commands.push_back(insert_command);
   db_ptr->set_SQL_commands(commands);
   db_ptr->execute_SQL_commands();
}

void insert_annotation(database* db_ptr)
{
   vector<string> commands;
   string insert_command="insert into annotation(altitude) ";
//   insert_command += "values('Peter','Cho','01821')";
//   insert_command += "values('Delsey','Sherrill','01821')";
//   insert_command += "values('Baby','Kermie','12345')";
   insert_command += "values(34.56)";
   cout << "insert_command = " << insert_command << endl;
   commands.push_back(insert_command);

//   string select_command="select * from annotation";
//   commands.push_back(select_command);

   db_ptr->set_SQL_commands(commands);
   db_ptr->execute_SQL_commands();
}

void select_annotations(database* db_ptr)
{
   vector<string> commands;

   string select_command = "SELECT id,timestamp,x(geometry) as Longitude,";
   select_command += "y(geometry) as Latitude,altitude as Altitude,label ";
   select_command += "from annotation";
//   cout << "select_command = " << select_command << endl;
   commands.push_back(select_command);

   db_ptr->set_SQL_commands(commands);
   db_ptr->execute_SQL_commands();
}

int main( int argc, char** argv )
{
   std::set_new_handler(sysfunc::out_of_memory);

   string hostname="localhost";
   string database_name="world_model";
//   string database_name="bpsimple";
   string username="cho";
   database db(hostname,database_name,username);

   db.connect();
//   insert_command(&db);
//   insert_annotation(&db);
   select_annotations(&db);

   db.disconnect();

/*    

   int nrows = PQntuples(res);
   cout << "Query resulted in: " << nrows << " results. " << endl;
    
   int nfields = PQnfields(res);
   cout << "Query results have " << nfields << " fields " << endl;

   char* value1 = PQgetvalue(res, 0, 0);
   cout << value1 << " ";
   char* value2 = PQgetvalue(res, 0, 1);
   cout << value2 << " " << endl;
*/

}

