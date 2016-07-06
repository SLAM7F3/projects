// =========================================================================
// Database class member function definitions
// =========================================================================
// Last modified on 8/14/10; 10/20/11; 1/11/12; 4/5/14
// =========================================================================

#include <iostream>
#include "postgres/database.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"

using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void database::allocate_member_objects()
{
}

void database::initialize_member_objects()
{
   connection_status_flag=false;
   password="postgres";

   db_connection_ptr=NULL;
   result_ptr=NULL;
   field_array_ptr=NULL;
}		 

// ---------------------------------------------------------------------
database::database()
{
   allocate_member_objects();
   initialize_member_objects();
}

// ---------------------------------------------------------------------
database::database(string host,string dbname,string user)
{
   hostname=host;
   database_name=dbname;
   username=user;

   allocate_member_objects();
   initialize_member_objects();
   connect();
}

// ---------------------------------------------------------------------
// Copy constructor:

database::database(const database& d)
{
   docopy(d);
}

database::~database()
{
//    cout << "inside database destructor" << endl;
   delete field_array_ptr;
   disconnect();
}

// ---------------------------------------------------------------------
void database::docopy(const database& d)
{
}

// Overload = operator:

database& database::operator= (const database& d)
{
   if (this==&d) return *this;
   docopy(d);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const database& d)
{
   outstream << endl;
   return(outstream);
}

// =========================================================================
// Postgres database connection member functions
// =========================================================================

bool database::connect()
{
//   cout << "inside database::connect()" << endl;
   string banner=
      "Opening connection to PostGres database "+database_name
      +" on host "+hostname+":";
   outputfunc::write_banner(banner);

   string connection_flags="host="+hostname;
   connection_flags += " dbname="+database_name;
   connection_flags += " user="+username;
   connection_flags += " password="+password;
//   cout << "connection_flags = " << connection_flags << endl;

   db_connection_ptr = PQconnectdb(connection_flags.c_str());   

// Set connection to nonblocking state to enable asynchronous
// communication with database while other main events take place:

   nonblocking_flag=(PQsetnonblocking(db_connection_ptr,1)==0);
//   cout << "nonblocking_flag = " << nonblocking_flag << endl;

   connection_status_flag=check_connection_status();

   if (!connection_status_flag)
   {
      cout << "Error !!!:  Cannot connect to "+database_name
         +" postgres database!" << endl;
   }

   return connection_status_flag;
}

// ---------------------------------------------------------------------
// Member function check_connection_status prints out a status
// message.  Note that libpq will always return a non-null PGconn*
// pointer except in a very low memory circumstances.  This boolean
// method returns true if a valid connection to the postgres database
// has been established.

bool database::check_connection_status()
{
//   cout << "inside check_connection_status()" << endl;
   connection_status_flag=false;
   if (db_connection_ptr != NULL)
   {
      connection_status = PQstatus(db_connection_ptr);
      print_connection_status();

      if (connection_status != CONNECTION_OK)
      {
         char* error_message=PQerrorMessage(db_connection_ptr);
         cout << "Error message = " << error_message << endl;
      }
      else
      {
         connection_status_flag=true;
      }
   }

//   cout << "connection_status_flag = " << connection_status_flag << endl;
   return connection_status_flag;
}

// ---------------------------------------------------------------------
void database::print_connection_status()
{
   string status_message="";
   switch (connection_status)
   {
      case CONNECTION_OK: 
         status_message="Connection established to Postgres database "
            +database_name+":";
         break;

      case CONNECTION_BAD:
         status_message="Bad connection to Postgres database "
            +database_name+":";
         break;

// Non-blocking mode only below here:
	    
// The existence of these should never be relied upon - they should
// only be used for user feedback or similar purposes.

         /* Waiting for connection to be made.  */
      case CONNECTION_STARTED:			
         status_message="Started";
         break;
	    
         /* Connection OK; waiting to send.	   */
      case CONNECTION_MADE:			
         status_message="Made";
         break;
	    
         /* Waiting for a response from the postmaster */
      case CONNECTION_AWAITING_RESPONSE:		
         status_message="Awaiting Response";
         break;
	    
         /* Received authentication; waiting for  backend startup. */
      case CONNECTION_AUTH_OK:	
         status_message="Auth OK";
         break;

         /* Negotiating environment. */
      case CONNECTION_SETENV:			
         status_message="Negotiating environment";
         break;
	    
      case CONNECTION_SSL_STARTUP: 
         status_message="SSL Startup (Negotiating SSL)";
         break;

         /* Internal state: connect() needed */
      case CONNECTION_NEEDED:			
         status_message="Needed (connect() needed)";
         break;

      default:
         status_message="Unknown Status";
   }

   outputfunc::write_banner(status_message);
}

// ---------------------------------------------------------------------
void database::disconnect()
{
   if (db_connection_ptr != NULL)
   {
      string banner="Disconnecting from PostGres database:";
      outputfunc::write_banner(banner);
      PQfinish(db_connection_ptr);
      db_connection_ptr = NULL;
   }
}

// =========================================================================
// PostgreSQL command handling functions
// =========================================================================

void database::set_SQL_commands(vector<string> commands)
{
//   cout << "inside database::set_SQL_commands(), this = " << this << endl;
   SQL_commands.clear();
   for (unsigned int n=0; n<commands.size(); n++)
   {
      SQL_commands.push_back(commands[n]);
   }
}

bool database::execute_SQL_commands()
{
//   cout << "inside database::execute_SQL_commands()" << endl;
//   cout << "this = " << this << endl;

   bool SQL_commands_executed=false;
   int max_counter_value=1000;
   if (connection_status_flag)
   {
      for (unsigned int n=0; n<SQL_commands.size(); n++)
      {
         int counter=0;
         do
         {
            query_sent_flag=send_query(SQL_commands[n]);
            counter++;
         }
         while(!query_sent_flag && counter < max_counter_value);
      
         if (!query_sent_flag)
         {
//            cout << "inside database::execute_SQL_commands(), query_sent_flag=0"
//                 << " counter = " << counter << endl;
//            outputfunc::enter_continue_char();
         }
         
//         cout << "n = " << n
//              <<  " SQL_command = " << SQL_commands[n] << endl;

         counter=0;
         do
         {
            ResultStatus=receive_results();
//            cout << "ResultStatus = " << ResultStatus << endl;
            if (ResultStatus==PGRES_TUPLES_OK && result_ptr != NULL)
            {
               parse_rows();
               SQL_commands_executed=true;
            }
            counter++;
         }
         while(result_ptr != NULL && counter < max_counter_value);

         if (result_ptr==NULL)
         {
//            cout << 
//               "inside database::execute_SQL_commands(), result_ptr=NULL" 
//                 << " counter = " << counter << endl;
//            outputfunc::enter_continue_char();
         }

         post_execute_SQL_command();
      }
   } // connection_status_flag conditional

//   cout << "SQL_commands_executed = " << SQL_commands_executed << endl;
   return SQL_commands_executed;
}

// ---------------------------------------------------------------------
// Member function select_data() takes in a single SQL select command.
// It returns Genarray<string>* field_array_ptr containing the data
// output response to the input query.

Genarray<string>* database::select_data(string curr_select_command)
{
//   cout << "inside database::select_data()" << endl;
   
   vector<string> select_commands;
   select_commands.push_back(curr_select_command);
   set_SQL_commands(select_commands);

   execute_SQL_commands();
   if (get_field_array_ptr()==NULL) 
   {
//      cout << "Trouble in database::select_data()!" << endl;
//      cout << "curr_select_cmd = " << endl;
//      cout << curr_select_command << endl;
//      cout << "No output returned from database!" << endl;
//      outputfunc::enter_continue_char();
   }
   return get_field_array_ptr();


}

// ---------------------------------------------------------------------
// Member function send_query asynchronously sends a query to the
// postgres database.  This boolean method returns false if the query
// was not successfully sent.

bool database::send_query(string command)
{
//   cout << "inside database::send_query, command = " << command << endl;
   
   if (connection_status_flag)
   {
      int send_query_status=PQsendQuery(db_connection_ptr,command.c_str());
//   cout << "send_query_status = " << send_query_status << endl;
      bool query_sent_flag=true;
      if (send_query_status != 1)
      {
         char* error_message=PQerrorMessage(db_connection_ptr);
         cout << "Send query error message = " << error_message << endl;
         query_sent_flag=false;
//      outputfunc::enter_continue_char();
      }
      return query_sent_flag;
   }
   else
   {
      return false;
   }
}

// ---------------------------------------------------------------------
// Member function receive_results asynchronously retrieves a non-null
// pointer for result_ptr and checks for result errors before
// attempting to actually perform the SQL call

ExecStatusType database::receive_results()
{
//   cout << "inside database::receive_results()" << endl;
   
   result_ptr=PQgetResult(db_connection_ptr);
   if (result_ptr != NULL)
   {
      string result_error_message=PQresultErrorMessage(result_ptr);
      if (result_error_message.size() > 0)
      {
         cout << "Result error message = " << PQresultErrorMessage(result_ptr)
              << endl;
         ResultStatus=PGRES_NONFATAL_ERROR;
      }
      else
      {
         ResultStatus=PQresultStatus(result_ptr);
      }
   }
   else
   {
//      cout << "result_ptr==NULL --> command has been processed" 
//           << endl;
   }

//   cout << "At end of database::receive_results(), Result status = " 
//        << PQresStatus(PQresultStatus(result_ptr)) << endl;

   return ResultStatus;
}

// ---------------------------------------------------------------------
// Member function post_execute_SQL_command retrieves clears member
// pointer result_ptr

void database::post_execute_SQL_command()
{
   PQclear(result_ptr);
//   result_ptr=NULL;
}

/*
// ---------------------------------------------------------------------
ExecStatusType database::determine_result_status()
{
//   cout << "SQL command result status = " 
//        << PQresStatus(PQresultStatus(result_ptr)) << endl;
//   cout << "# rows affected = " << PQcmdTuples(result_ptr) << endl;

   ResultStatus=PQresultStatus(result_ptr);

   switch (ResultStatus)
   {
      case PGRES_TUPLES_OK: 
         
// May have some data to process, find out:

         parse_rows();

         break;

// No data, drop through to no data case:

      case PGRES_COMMAND_OK:

// All OK, no data to process:

         break;

      case PGRES_EMPTY_QUERY:
// Server had nothing to do, a bug maybe?

         break;
         
      case PGRES_NONFATAL_ERROR:
// Can continue, possibly retry the command

         break;

      case PGRES_BAD_RESPONSE:
      case PGRES_FATAL_ERROR:
      default:

         break;
// Fatal or unknown error, cannot continue

   } // switch statement

   return ResultStatus;
}
*/

// ---------------------------------------------------------------------
// Member function parse_rows loops over every column (field) within
// every row in the postgres database.  

int database::parse_rows()
{
//   cout << "inside database::parse_rows()" << endl;
   n_rows=PQntuples(result_ptr);
   n_columns=PQnfields(result_ptr);
//   cout << "n_rows = " << n_rows << " n_columns = " << n_columns << endl;

   delete field_array_ptr;
   field_array_ptr=NULL;

   if (n_rows==0)
   {
//      cout << "n_rows = 0 !!!!!" << endl;
      return n_rows;
   }

   field_array_ptr=new Genarray<string>(n_rows,n_columns);
//   cout << "field_array_ptr = " << field_array_ptr << endl;

   for (int r=0; r<n_rows; r++)
   {
//      cout << "Row = " << r << endl;

      vector<string> field_name;
      vector<int> field_size;
      vector<int> field_index;
      for (int c=0; c<n_columns; c++)
      {
         field_name.push_back(PQfname(result_ptr,c));
         field_size.push_back(PQfsize(result_ptr,c));
         field_index.push_back(PQfnumber(
            result_ptr,field_name.back().c_str()));
         string field_value=parse_value(r,c);
//         cout << " field_name = " << field_name.back()
//              << " field_size = " << field_size.back()
//              << " field_index = " << field_index.back() 
//              << " field_value = " << field_value << endl;
         field_array_ptr->put(r,c,field_value);
         
      } // loop over index c labeling fields in columns
//      cout << endl;
   } // loop over index r labeling rows


//   cout << "*field_array_ptr = " << *field_array_ptr << endl;

   return n_rows;
}

// ---------------------------------------------------------------------
string database::parse_value(int row,int column)
{
   string value_str="NULL";
   if (PQgetisnull(result_ptr,row,column))
   {
   }
   else
   {
      value_str=PQgetvalue(result_ptr,row,column);
   }
   return value_str;
}

// ---------------------------------------------------------------------
// Member function fetch_rows_in_batches enters into a while loop and
// processes nrows_to_fetch worth of postgres database rows at a time.

void database::fetch_rows_in_batches(
   int nrows_to_fetch,string cursor_name)
{
   int batch_number=0;
   string command="FETCH "+stringfunc::number_to_string(nrows_to_fetch)
      +" in "+cursor_name;

   do
   {
      do
      {
         query_sent_flag=send_query(command);
      }
      while(!query_sent_flag);

      do
      {
         ResultStatus=receive_results();
         if (ResultStatus==PGRES_TUPLES_OK && result_ptr != NULL)
         {
            parse_rows();
            batch_number++;
            cout << "--------------------------------------------------"
                 << endl;
         }
         post_execute_SQL_command();
      }
      while(result_ptr != NULL);
   }
   while (n_rows > 0);
}

// ---------------------------------------------------------------------
// Member function get_next_id takes in the name for some postgres
// database sequence.  It queries the database for the sequence's next
// ID and returns its value.

int database::get_next_id(string sequence)
{
   vector<string> commands;
   commands.push_back("SELECT nextval('"+sequence+"')");
   set_SQL_commands(commands);
   execute_SQL_commands();

   int next_id=-1;
   if (field_array_ptr->get_mdim()==1 && field_array_ptr->get_ndim()==1)
   {
      next_id=stringfunc::string_to_integer(field_array_ptr->get(0,0));
   }
   else
   {
      cout << "Error in database::get_next_id()" << endl;
      cout << "*field_array_ptr = " << *field_array_ptr << endl;
      exit(-1);
   }
   return next_id;
}

// ---------------------------------------------------------------------
// Member function get_n_table_rows() takes in the name for some table
// within the current database.  It counts and returns the total
// number of rows in the table.

int database::get_n_table_rows(string table_name)
{
//   cout << "inside database::get_n_table_rows()" << endl;

   string SQL_command="select count(*) from "+table_name+" ;";
//   cout << "SQL_command = " << SQL_command << endl;

   vector<string> sql_commands;
   sql_commands.push_back(SQL_command);
//   cout << "sql_commands.back() = " << sql_commands.back() << endl;
   set_SQL_commands(sql_commands);
   execute_SQL_commands();

   int n_table_rows=stringfunc::string_to_number(
      get_field_array_ptr()->get(0,0));
//   cout << "n_table_rows = " << n_table_rows << endl;
   return n_table_rows;
}
